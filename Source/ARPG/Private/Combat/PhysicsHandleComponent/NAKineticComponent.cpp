// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/PhysicsHandleComponent/NAKineticComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "NACharacter.h"
#include "Ability/GameInstanceSubsystem/NAAbilityGameInstanceSubsystem.h"
#include "Combat/AttributeSet/NAKineticAttributeSet.h"
#include "Combat/GameplayAbility/NAGA_KineticGrab.h"
#include "Combat/GameplayEffect/NAGE_KineticAP.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UNAKineticComponent::UNAKineticComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault( true );

	// ...
	ConstraintInstance.EnableParentDominates();
	ConstraintInstance.SetDisableCollision( true );
	ConstraintInstance.ProfileInstance.LinearBreakThreshold = 2000.f;
	ConstraintInstance.ProfileInstance.AngularBreakThreshold = 2000.f;

	InterpolationSpeed = 20.0f;
	LinearStiffness = 3000.0f;
	LinearDamping = 500.0f;

	AngularStiffness = 1500.0f;
	AngularDamping = 200.0f;

	bSoftAngularConstraint = true;
	bSoftLinearConstraint = true;
}

void UNAKineticComponent::Grab()
{
	if ( const TScriptInterface<IAbilitySystemInterface>& Interface = GetOwner() )
	{
		Interface->GetAbilitySystemComponent()->AbilityLocalInputPressed( static_cast<int32>( EAbilityInputID::Grab ) );
	}
}

void UNAKineticComponent::Release()
{
	if ( !bIsGrab )
	{
		return;
	}
	
	if ( const TScriptInterface<IAbilitySystemInterface>& Interface = GetOwner() )
	{
		Interface->GetAbilitySystemComponent()->AbilityLocalInputReleased( static_cast<int32>( EAbilityInputID::Grab ) );
	}
}

float UNAKineticComponent::GetRange() const
{
	if ( const UNAKineticAttributeSet* Set = GetAttributeSet() )
	{
		return Set->GetRange();
	}

	return 0.f;
}

float UNAKineticComponent::GetMinHoldRange() const
{
	if ( const UNAKineticAttributeSet* Set = GetAttributeSet() )
	{
		return Set->GetMinHoldRange();
	}

	return 0.f;
}

float UNAKineticComponent::GetMaxHoldRange() const
{
	if ( const UNAKineticAttributeSet* Set = GetAttributeSet() )
	{
		return Set->GetMaxHoldRange();
	}

	return 0.f;
}

float UNAKineticComponent::GetForce() const
{
	if ( const UNAKineticAttributeSet* Set = GetAttributeSet() )
	{
		return Set->GetForce();
	}

	return 0.f;
}

float UNAKineticComponent::GetGrabDistance() const
{
	return GrabDistance;
}

FVector_NetQuantizeNormal UNAKineticComponent::GetActorForward() const
{
	return ActorForward;
}

void UNAKineticComponent::ToggleGrabAbility( const bool bFlag )
{
	if ( bFlag )
	{
		if( !GrabSpecHandle.IsValid() )
		{
			if ( const TScriptInterface<IAbilitySystemInterface>& Interface = GetOwner() )
			{
				const FGameplayAbilitySpec Spec( UNAGA_KineticGrab::StaticClass(), 1.f, static_cast<int32>( EAbilityInputID::Grab ) );
				GrabSpecHandle = Interface->GetAbilitySystemComponent()->GiveAbility( Spec );
			}	
		}
	}
	else
	{
		if ( const TScriptInterface<IAbilitySystemInterface>& Interface = GetOwner() )
		{
			Interface->GetAbilitySystemComponent()->ClearAbility( GrabSpecHandle );
			GrabSpecHandle = {};
		}
	}
}

void UNAKineticComponent::ForceUpdateActorForward()
{
	if ( const APawn* OwnerPawn = Cast<APawn>( GetOwner() ) )
	{
		if ( const APlayerController* PlayerController = Cast<APlayerController>( OwnerPawn->GetController() ) )
		{
			ActorForward = PlayerController->GetControlRotation().Vector();
		}
	}
}

const UNAKineticAttributeSet* UNAKineticComponent::GetAttributeSet() const
{
	if ( const TScriptInterface<IAbilitySystemInterface>& Interface = GetOwner() )
	{
		return Cast<UNAKineticAttributeSet>( Interface->GetAbilitySystemComponent()->GetAttributeSet( UNAKineticAttributeSet::StaticClass() ) );
	}

	return nullptr;
}

// Called when the game starts
void UNAKineticComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	check( Cast<ANACharacter>( GetOwner() ) );
	
	if ( const ANACharacter* Character = Cast<ANACharacter>( GetOwner() );
		 Character && Character->HasAuthority() )
	{
		const UNAAbilityGameInstanceSubsystem* AbilityGameInstance = GetWorld()->GetGameInstance()->GetSubsystem<UNAAbilityGameInstanceSubsystem>(); 
		Character->GetAbilitySystemComponent()->InitStats( UNAKineticAttributeSet::StaticClass(), AbilityGameInstance->GetKineticAttributesDataTable() );
	}

	GrabDistance = GetMinHoldRange();
}

// Called every frame
void UNAKineticComponent::TickComponent( float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...

	if ( GetOwner()->HasAuthority() )
	{
		if ( !bIsGrab )
		{
			if ( const TScriptInterface<IAbilitySystemInterface>& Interface = GetOwner() )
			{
				if ( const UNAKineticAttributeSet* AttributeSet = Cast<UNAKineticAttributeSet>( Interface->GetAbilitySystemComponent()->GetAttributeSet( UNAKineticAttributeSet::StaticClass() ) ) )
				{
					if ( AttributeSet->GetAP() + DeltaTime < AttributeSet->GetMaxAP() )
					{
						const FGameplayEffectContextHandle Context = Interface->GetAbilitySystemComponent()->MakeEffectContext();
						const FGameplayEffectSpecHandle SpecHandle = Interface->GetAbilitySystemComponent()->MakeOutgoingSpec( UNAGE_KineticAP::StaticClass(), 1.f, Context );
						SpecHandle.Data->SetSetByCallerMagnitude( FGameplayTag::RequestGameplayTag( "Data.KineticAP" ), 10.f * DeltaTime );
						const FActiveGameplayEffectHandle Handle = Interface->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf( *SpecHandle.Data.Get() );
						check( Handle.WasSuccessfullyApplied() );
					}
				}
			}
		}
		else
		{
			if ( const APawn* OwnerPawn = Cast<APawn>( GetOwner() ) )
			{
				if ( const APlayerController* PlayerController = Cast<APlayerController>( OwnerPawn->GetController() ) )
				{
					ActorForward = PlayerController->GetControlRotation().Vector();
				}
			}
		}
	}
}

void UNAKineticComponent::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
	DOREPLIFETIME( UNAKineticComponent, bIsGrab );
}

