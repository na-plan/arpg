// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/PhysicsHandleComponent/NAKineticComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
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

	FIND_OBJECT( GreenMaterial, "/Script/Engine.MaterialInstanceConstant'/Game/00_ProjectNA/05_Resource/01_Material/Issac/MI_health_green.MI_health_green'")
	FIND_OBJECT( YellowMaterial, "/Script/Engine.MaterialInstanceConstant'/Game/00_ProjectNA/05_Resource/01_Material/Issac/MI_health_yellow.MI_health_yellow'")
	FIND_OBJECT( RedMaterial, "/Script/Engine.MaterialInstanceConstant'/Game/00_ProjectNA/05_Resource/01_Material/Issac/MI_health_red.MI_health_red'")
	FIND_OBJECT( GrabAction, "/Script/EnhancedInput.InputAction'/Game/00_ProjectNA/01_Blueprint/03_Actions/IA_Kinetic/IA_KineticGrab.IA_KineticGrab'" )
	FIND_OBJECT( ThrowAction, "/Script/EnhancedInput.InputAction'/Game/00_ProjectNA/01_Blueprint/03_Actions/IA_Kinetic/IA_KineticThrow.IA_KineticThrow'")
	FIND_OBJECT( DistanceAdjustAction, "/Script/EnhancedInput.InputAction'/Game/00_ProjectNA/01_Blueprint/03_Actions/IA_Kinetic/IA_KineticDistance.IA_KineticDistance'")
	FIND_OBJECT( KineticMappingContext, "/Script/EnhancedInput.InputMappingContext'/Game/00_ProjectNA/01_Blueprint/02_Inputs/IMC_Kinetic.IMC_Kinetic'")
}

void UNAKineticComponent::Grab()
{
	if ( bIsGrab )
	{
		return;
	}
	
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

bool UNAKineticComponent::HasGrabbed() const
{
	return bIsGrab;
}

void UNAKineticComponent::BindKineticKeys()
{
	if ( const APawn* OwnerPawn = Cast<APawn>( GetOwner() ) )
	{
		if ( const APlayerController* PlayerController = Cast<APlayerController>( OwnerPawn->GetController() ) )
		{
			if ( UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>( PlayerController->GetLocalPlayer() ) )
			{
				Subsystem->AddMappingContext( KineticMappingContext, 2 );
			}
		}
		
		if ( UEnhancedInputComponent* InputComponent = Cast<UEnhancedInputComponent>( OwnerPawn->InputComponent ) )
		{
			GrabActionBinding = &InputComponent->BindAction( GrabAction, ETriggerEvent::Started, this, &UNAKineticComponent::Grab );
			ReleaseActionBinding = &InputComponent->BindAction( GrabAction, ETriggerEvent::Completed, this, &UNAKineticComponent::Release );
			ThrowActionBinding = &InputComponent->BindAction( ThrowAction, ETriggerEvent::Started, this, &UNAKineticComponent::Throw );
			DistanceAdjustActionBinding = &InputComponent->BindAction( ThrowAction, ETriggerEvent::Started, this, &UNAKineticComponent::AdjustDistance );
		}
	}
}

void UNAKineticComponent::UnbindKineticKeys()
{
	if ( const APawn* OwnerPawn = Cast<APawn>( GetOwner() ) )
	{
		if ( const APlayerController* PlayerController = Cast<APlayerController>( OwnerPawn->GetController() ) )
		{
			if ( UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>( PlayerController->GetLocalPlayer() ) )
			{
				Subsystem->RemoveMappingContext( KineticMappingContext );
			}
		}
		
		if ( UEnhancedInputComponent* InputComponent = Cast<UEnhancedInputComponent>( OwnerPawn->InputComponent ) )
		{
			if ( GrabActionBinding )
			{
				InputComponent->RemoveBinding( *GrabActionBinding );
				InputComponent->RemoveBinding( *ReleaseActionBinding );

				GrabActionBinding = nullptr;
				ReleaseActionBinding = nullptr;
			}
			if ( ThrowActionBinding )
			{
				InputComponent->RemoveBinding( *ThrowActionBinding );
				ThrowActionBinding = nullptr;
			}

			if ( DistanceAdjustActionBinding )
			{
				InputComponent->RemoveBinding( *DistanceAdjustActionBinding );
				DistanceAdjustActionBinding = nullptr;
			}
		}
	}
}

void UNAKineticComponent::OnAPChanged( const FOnAttributeChangeData& OnAttributeChangeData )
{
	OnAPChanged( OnAttributeChangeData.OldValue, OnAttributeChangeData.NewValue );
}

void UNAKineticComponent::OnAPChanged( float Old, float New )
{
	if ( const TScriptInterface<IAbilitySystemInterface>& Interface = GetOwner() )
	{
		if ( const UNAKineticAttributeSet* AttributeSet = Cast<UNAKineticAttributeSet>( Interface->GetAbilitySystemComponent()->GetAttributeSet( UNAKineticAttributeSet::StaticClass() ) ) )
		{
			const float MaxAP = AttributeSet->GetMaxAP();

			static constexpr int32 MaxHealthMesh = 4;
	
			const float NewRatio = New / MaxAP;
			const int32 FillCount = NewRatio <= 0 ? 0 : static_cast<int32>( NewRatio / MeshHealthStep );
			check( FillCount <= MaxHealthMesh );
	
			UMaterialInstance* TargetMaterial = nullptr;

			if ( FillCount >= 4 )
			{
				TargetMaterial = GreenMaterial;
			}
			else if ( FillCount >= 3 )
			{
				TargetMaterial = YellowMaterial;
			}
			else if ( FillCount >= 2 )
			{
				TargetMaterial = RedMaterial;
			}

			if ( USkeletalMeshComponent* SkeletalMeshComponent = GetOwner()->GetComponentByClass<USkeletalMeshComponent>() )
			{
				constexpr const char* HealthMaterial = "health";
				SkeletalMeshComponent->SetMaterialByName( HealthMaterial, TargetMaterial );
			}
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

		if ( GetOwner()->HasAuthority() )
		{
			Character->GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate( UNAKineticAttributeSet::GetAPAttribute() ).AddUObject( this, &UNAKineticComponent::OnAPChanged );	
		}
		else
		{
			if ( const UNAKineticAttributeSet* AttributeSet = Cast<UNAKineticAttributeSet>( Character->GetAbilitySystemComponent()->GetAttributeSet( UNAKineticAttributeSet::StaticClass() )  ))
			{
				AttributeSet->OnAPChanged.AddUObject( this, &UNAKineticComponent::OnAPChanged );
			}
		}
	}

	GrabDistance = GetMinHoldRange();

	if ( const APawn* Pawn = Cast<APawn>( GetOwner() );
		 Pawn && Pawn->IsLocallyControlled() )
	{
		BindKineticKeys();
	}
}

void UNAKineticComponent::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
	Super::EndPlay( EndPlayReason );

	if ( const APawn* Pawn = Cast<APawn>( GetOwner() );
		 Pawn && Pawn->IsLocallyControlled() )
	{
		UnbindKineticKeys();
	}
}

void UNAKineticComponent::Throw()
{
	if ( !bIsGrab )
	{
		return;
	}
	
	if ( GetOwner()->HasAuthority() )
	{
		ThrowImpl();
	}
	else
	{
		Server_Throw();
	}
}

void UNAKineticComponent::ThrowImpl() const
{
	if ( GetNetMode() == NM_Client )
	{
		return;
	}
	
	if ( const TScriptInterface<IAbilitySystemInterface>& Interface = GetOwner() )
	{
		if ( GrabSpecHandle.IsValid() )
		{
			if ( const FGameplayAbilitySpec* Spec =  Interface->GetAbilitySystemComponent()->FindAbilitySpecFromHandle( GrabSpecHandle ) )
			{
				for ( UGameplayAbility* Ability : Spec->GetAbilityInstances() )
				{
					if ( UNAGA_KineticGrab* GrabInstance = Cast<UNAGA_KineticGrab>( Ability ) )
					{
						GrabInstance->Throw();	
					}
				}
			}
		}
	}
}

void UNAKineticComponent::Server_Throw_Implementation()
{
	ThrowImpl();
}

void UNAKineticComponent::AdjustDistance( const FInputActionValue& InputActionValue )
{
	
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

