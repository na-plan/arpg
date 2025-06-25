// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/ActorComponent/NACombatComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "ARPG/ARPG.h"
#include "Net/UnrealNetwork.h"
#include "Combat/Interface/NAHandActor.h"

DEFINE_LOG_CATEGORY( LogCombatComponent );

// Sets default values for this component's properties
UNACombatComponent::UNACombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;

	// ...
}


void UNACombatComponent::SetAttackAbility(const TSubclassOf<UGameplayAbility>& InAbility)
{
	AttackAbility = InAbility;
	UpdateAttackAbilityToASC( false );
}

void UNACombatComponent::SetGrabAbility(const TSubclassOf<UGameplayAbility>& InAbility)
{
	if ( const TScriptInterface<IAbilitySystemInterface>& Interface = GetAttacker() )
	{
		if (AbilitySpecHandle.IsValid() && GetNetMode() != NM_Client)
		{
			Interface->GetAbilitySystemComponent()->ClearAbility(AbilitySpecHandle);
		}

		if (InAbility && GetNetMode() != NM_Client)
		{
			FGameplayAbilitySpec Spec( InAbility, 1.f );
			AbilitySpecHandle = Interface->GetAbilitySystemComponent()->GiveAbility(InAbility);
		}
	}

	GrabAbility = InAbility;
}

TSubclassOf<UGameplayEffect> UNACombatComponent::GetAmmoType() const
{
	return AmmoType;
}

void UNACombatComponent::OnAbilityEnded( const FAbilityEndedData& AbilityEndedData )
{
	if ( AbilityEndedData.AbilityThatEnded->IsA( AttackAbility ) )
	{
		if ( !AbilityEndedData.bWasCancelled )
		{
			if ( bReplay && bAttacking )
			{
				UE_LOG( LogTemp, Log, TEXT("[%hs]: Ability replay from %d"), __FUNCTION__, GetNetMode() )
				StartAttack();
			}
		}
		else
		{
			UE_LOG( LogTemp, Log, TEXT("[%hs]: Ability cancelled"), __FUNCTION__ )
		}
	}
}

// Called when the game starts
void UNACombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	DoStartAttack.AddUniqueDynamic(this, &UNACombatComponent::StartAttack);
	DoStopAttack.AddUniqueDynamic(this, &UNACombatComponent::StopAttack);
	bCanAttack = IsAbleToAttack();

	// 클라이언트의 BeginPlay에 맞춰서 초기화
	if ( const APawn* Attacker = GetAttacker() )
	{
		if ( Attacker->IsLocallyControlled() )
		{
			Server_RequestAttackAbility();
			
			if ( const TScriptInterface<IAbilitySystemInterface>& Interface = GetAttacker() )
			{
				Interface->GetAbilitySystemComponent()->OnAbilityEnded.AddUObject( this, &UNACombatComponent::OnAbilityEnded );
			}
		}
	}
}

void UNACombatComponent::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
	Super::EndPlay( EndPlayReason );
	UpdateAttackAbilityToASC( true );
}

void UNACombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION( UNACombatComponent, bCanAttack, COND_OwnerOnly );
	DOREPLIFETIME_CONDITION( UNACombatComponent, AttackOrientation, COND_OwnerOnly );
}

void UNACombatComponent::Server_SyncAttack_Implementation( const bool bFlag )
{
	bAttacking = bFlag;
}

void UNACombatComponent::SetActive( bool bNewActive, bool bReset )
{
	bool bPrevious = IsActive();
	Super::SetActive( bNewActive, bReset );

	if ( bPrevious != bNewActive )
	{
		if ( !bNewActive )
		{
			StopAttack();
			UpdateAttackAbilityToASC( true );
		}
		else
		{
			UpdateAttackAbilityToASC( false );
		}
	}
}

bool UNACombatComponent::IsAbleToAttack()
{
	// Ammo, stamina, montage duration, etc...
	bool bResult = AttackAbility != nullptr && IsActive();

	if ( GetNetMode() != NM_Client )
	{
		if ( bResult )
		{
			const TScriptInterface<IAbilitySystemInterface>& Interface = GetAttacker();
			bResult &= Interface != nullptr;

			if ( Interface )
			{
				const FGameplayAbilitySpec* Spec = Interface->GetAbilitySystemComponent()->FindAbilitySpecFromHandle( AbilitySpecHandle );
				bResult &= Spec != nullptr;

				if ( Spec )
				{
					bResult &= !Spec->IsActive();
				}
			}
		}
	}
	
	return bResult;
}

void UNACombatComponent::SetAttack(const bool NewAttack)
{
	if ( !bCanAttack && NewAttack )
	{
		return;
	}

	if ( bAttacking == NewAttack )
	{
		return;
	}

	if ( !AttackAbility )
	{
		return;
	}
	
	UE_LOG( LogCombatComponent, Log, TEXT("%hs: Attack from %d to %d by %d"), __FUNCTION__, bAttacking, NewAttack, GetNetMode() );

	bAttacking = NewAttack;
	if ( GetNetMode() == NM_Client )
	{
		Server_SyncAttack( bAttacking );
	}
}

void UNACombatComponent::UpdateAttackAbilityToASC( const bool bOnlyRemove )
{
	UWorld* World = GetWorld();
	
	if ( const TScriptInterface<IAbilitySystemInterface>& Interface = GetAttacker();
		 World && Interface && World->IsGameWorld() )
	{
		if ( AbilitySpecHandle.IsValid() && GetNetMode() != NM_Client )
		{
			Interface->GetAbilitySystemComponent()->ClearAbility( AbilitySpecHandle );
		}

		if ( !bOnlyRemove && AttackAbility && GetNetMode() != NM_Client )
		{
			AbilitySpecHandle = Interface->GetAbilitySystemComponent()->GiveAbility( AttackAbility );		
		}
	}
}

void UNACombatComponent::StartAttack()
{
	UE_LOG( LogCombatComponent, Log, TEXT("%hs: Try attack, Is Client?: %d"), __FUNCTION__, GetNetMode() == NM_Client );

	if ( !IsActive() )
	{
		return;
	}
	
	bCanAttack = IsAbleToAttack();

	if ( !bCanAttack )
	{
		SetAttack( false );
		StopAttack();
	}

	UpdateAttackOrientation();
	
	// 공격을 수행하고
	if ( const TScriptInterface<IAbilitySystemInterface> Interface = GetAttacker();
		 Interface && Interface->GetAbilitySystemComponent()->TryActivateAbilityByClass( AttackAbility ) )
	{
		SetAttack( true );
	}
	else
	{
		// Commit Ability에서 실패할 경우에도 공격을 중단
		SetAttack( false );
	}
}

APawn* UNACombatComponent::GetAttacker() const
{
	if ( bConsiderChildActor )
	{
		if ( APawn* Pawn = Cast<APawn>( GetOwner()->GetAttachParentActor() ) )
		{
			return Pawn;
		}
		if ( const USceneComponent* ParentComponent = GetOwner()->GetParentComponent() )
		{
			if ( APawn* ComponentPawn = Cast<APawn>( ParentComponent->GetOwner() ) )
			{
				return ComponentPawn;
			}
		}
	}
	
	return Cast<APawn>( GetOwner() );
}

void UNACombatComponent::StopAttack()
{
	bAttacking = false;

	if ( GetNetMode() == NM_Client )
	{
		Server_SyncAttack( bAttacking );
	}
}

void UNACombatComponent::OnRep_CanAttack()
{
	if ( !bCanAttack && bAttacking )
	{
		StopAttack();
	}
}

void UNACombatComponent::UpdateAttackOrientation()
{
	AttackOrientation = GetAttacker()->GetControlRotation();
	Server_CommitAttackOrientation( GetAttacker()->GetControlRotation() );
}

void UNACombatComponent::SetConsiderChildActor(const bool InConsiderChildActor)
{
	bConsiderChildActor = InConsiderChildActor;
}

TSubclassOf<UGameplayAbility> UNACombatComponent::GetAttackAbility() const
{
	return AttackAbility;
}

FRotator UNACombatComponent::GetAttackOrientation() const
{
	return AttackOrientation;
}

void UNACombatComponent::Server_RequestAttackAbility_Implementation()
{
	UpdateAttackAbilityToASC( false );
}

void UNACombatComponent::Server_CommitAttackOrientation_Implementation( const FRotator& Rotator )
{
	AttackOrientation = Rotator;
}

// Called every frame
void UNACombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

