// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/ActorComponent/NACombatComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
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
	if (const TScriptInterface<IAbilitySystemInterface>& Interface = GetAttacker())
	{
		if (AbilitySpecHandle.IsValid() && GetNetMode() != NM_Client)
		{
			Interface->GetAbilitySystemComponent()->ClearAbility(AbilitySpecHandle);
		}

		if (InAbility && GetNetMode() != NM_Client)
		{
			AbilitySpecHandle = Interface->GetAbilitySystemComponent()->GiveAbility(InAbility);
		}
	}

	GrabAbility = InAbility;
}

void UNACombatComponent::SetZoomAbility(const TSubclassOf<UGameplayAbility>& InAbility)
{
}

TSubclassOf<UGameplayEffect> UNACombatComponent::GetAmmoType() const
{
	return AmmoType;
}

void UNACombatComponent::ReplayAttack()
{
	bCanAttack = IsAbleToAttack();
	
	if (bCanAttack && bAttacking)
	{
		OnAttack();
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
		if ( Attacker->GetController() == GetWorld()->GetFirstPlayerController() )
		{
			Server_RequestAttackAbility();
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
	DOREPLIFETIME_CONDITION(UNACombatComponent, bCanAttack, COND_OwnerOnly)
}

void UNACombatComponent::SetActive( bool bNewActive, bool bReset )
{
	Super::SetActive( bNewActive, bReset );
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

bool UNACombatComponent::IsAbleToAttack()
{
	// Ammo, stamina, montage duration, etc...
	return AttackAbility != nullptr && IsActive();
}

bool UNACombatComponent::IsAbleToZoom()
{
	return false;
}

void UNACombatComponent::SetAttack(const bool NewAttack)
{
	if (!bCanAttack && NewAttack)
	{
		return;
	}

	if (bAttacking == NewAttack)
	{
		return;
	}

	if (!AttackAbility)
	{
		return;
	}

	
	UE_LOG(LogCombatComponent, Log, TEXT("%hs: Attack from %d to %d by %d"), __FUNCTION__, bAttacking, NewAttack, GetNetMode());
	bAttacking = NewAttack;
	
	if (GetNetMode() == NM_Client)
	{
		// 클라이언트일 경우 서버와 동기화 요청
		Server_SetAttack(bAttacking);
	}
	else if (GetNetMode() == NM_Standalone)
	{
		// 로컬 플레이일 경우 후처리 수행
		PostSetAttack();
	}
	else
	{
		// 서버에서 발생한 경우 클라이언트와 동기화
		Client_SyncAttack(bAttacking);
		PostSetAttack();
	}
}

void UNACombatComponent::UpdateAttackAbilityToASC( const bool bOnlyRemove )
{
	UWorld* World = GetWorld();
	
	if ( const TScriptInterface<IAbilitySystemInterface>& Interface = GetOwner();
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

void UNACombatComponent::SetZooming(bool NewZoom)
{

}

void UNACombatComponent::PostSetAttack()
{
	if (bAttacking)
	{
		// 공격으로 변환했다면 공격 루틴 시작
		OnAttack();
	}
	else
	{
		// 아니라면 대기 초기화
		if (AttackTimerHandler.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandler);	
		}
	}
}

//void UNACombatComponent::Server_SetZoom_Implementation(const bool NewAttack)
//{
//}
//void UNACombatComponent::Server_SetZoom_Validate(const bool NewAttack)
//{
//}

//void UNACombatComponent::Client_SyncZoom_Implementation(const bool NewFire)
//{
//}

void UNACombatComponent::Server_SetAttack_Implementation(const bool NewAttack)
{
	bCanAttack = IsAbleToAttack();
	if (NewAttack == bAttacking)
	{
		return;
	}
	
	if (!bCanAttack && NewAttack)
	{
		Client_SyncAttack(false);
		return;
	}
	
	bAttacking = NewAttack;
	PostSetAttack();
}

bool UNACombatComponent::Server_SetAttack_Validate(const bool NewAttack)
{
	return true;
}


void UNACombatComponent::Client_SyncAttack_Implementation(const bool NewFire)
{
	bAttacking = NewFire;
}

void UNACombatComponent::StartAttack()
{
	UE_LOG(LogCombatComponent, Log, TEXT("%hs: Try attack"), __FUNCTION__);

	if ( !IsActive() )
	{
		return;
	}
	
	bCanAttack = IsAbleToAttack();
	if (bCanAttack && !bAttacking)
	{
		SetAttack(true);
	}
}

APawn* UNACombatComponent::GetAttacker() const
{
	if ( bConsiderChildActor )
	{
		return Cast<APawn>( GetOwner()->GetAttachParentActor() );
	}
	
	return Cast<APawn>( GetOwner() );
}

void UNACombatComponent::OnAttack()
{
	if (GetNetMode() != NM_Client)
	{
		bCanAttack = IsAbleToAttack();

		// 이번 회차에서 공격을 할 수 없다면 공격 중단
		if (!bCanAttack)
		{
			UE_LOG(LogCombatComponent, Log, TEXT("Unable to attack, bCanAttack is false"));
			StopAttack();
			return;
		}

		// 공격이 가능하고 공격을 시도하는 중이라면
		if (bCanAttack && bAttacking&& !bCanGrab)
		{
			// 공격을 수행하고
			if ( const TScriptInterface<IAbilitySystemInterface> Interface = GetOwner();
				 Interface && Interface->GetAbilitySystemComponent()->TryActivateAbilityByClass( AttackAbility ) )
			{
				OnAttack_Implementation();

				// 만약 재수행이 설정돼 있다면 Timer로 예약
				if (bReplay)
				{
					const float NextTime = GetNextAttackTime();
					GetWorld()->GetTimerManager().SetTimer
					(
						AttackTimerHandler,
						this,
						&UNACombatComponent::ReplayAttack,
						NextTime,
						true
					);
				}
				else
				{
					// 아니라면 공격을 강제 중단
					SetAttack(false);	
				}
			}
			else
			{
				// Commit Ability에서 실패할 경우에도 공격을 중단
				SetAttack(false);
			}
		}
		//잡기 가능하면
		else if (bAttacking && bCanGrab)
		{
			if (const TScriptInterface<IAbilitySystemInterface> Interface = GetOwner();
				Interface && Interface->GetAbilitySystemComponent()->TryActivateAbilityByClass(AttackAbility))
			{
				OnAttack_Implementation();

				// 만약 재수행이 설정돼 있다면 Timer로 예약
				if (bReplay)
				{
					const float NextTime = GetNextAttackTime();
					GetWorld()->GetTimerManager().SetTimer
					(
						AttackTimerHandler,
						this,
						&UNACombatComponent::ReplayAttack,
						NextTime,
						true
					);
				}
				else
				{
					// 아니라면 공격을 강제 중단
					SetAttack(false);
				}
			}
			else
			{
				// Commit Ability에서 실패할 경우에도 공격을 중단
				SetAttack(false);
			}
		}
	}
}

void UNACombatComponent::OnRep_CanAttack()
{
	if (!bCanAttack && bAttacking)
	{
		StopAttack();
	}
}

void UNACombatComponent::SetConsiderChildActor(const bool InConsiderChildActor)
{
	bConsiderChildActor = InConsiderChildActor;
}

TSubclassOf<UGameplayAbility> UNACombatComponent::GetAttackAbility() const
{
	return AttackAbility;
}

void UNACombatComponent::Server_RequestZoom_Implementation()
{
	//SetZoomAbility(ZoomAbility);
}

void UNACombatComponent::Server_RequestAttackAbility_Implementation()
{
	UpdateAttackAbilityToASC( false );
}

void UNACombatComponent::StopAttack()
{
	if (bAttacking)
	{
		SetAttack( false );
	}
}

void UNACombatComponent::ZoomToggle()
{

}

// Called every frame
void UNACombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

