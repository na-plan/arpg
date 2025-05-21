// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/ActorComponent/NACombatComponent.h"

#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UNACombatComponent::UNACombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UNACombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UNACombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UNACombatComponent, bCanAttack, COND_OwnerOnly)
}

void UNACombatComponent::UpdateCanAttack()
{
	// Ammo, stamina, montage duration, etc...
	if (GetNetMode() != NM_Client)
	{
		bool bResult = true;

		for (const auto& Predication : Predications)
		{
			bResult |= Predication(GetOwner());
			if (!bResult)
			{
				break;
			}
		}

		bCanAttack = bResult;
	}
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

	bAttacking = NewAttack;
	Server_SetAttack(bAttacking);
}

void UNACombatComponent::Server_SetAttack_Implementation(const bool NewAttack)
{
	UpdateCanAttack();

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
}

bool UNACombatComponent::Server_SetAttack_Validate(const bool NewAttack)
{
	return true;
}


void UNACombatComponent::Client_SyncAttack_Implementation(const bool NewFire)
{
	bAttacking = NewFire;
}

void UNACombatComponent::OnAttack()
{
	if (GetNetMode() != NM_Client && bCanAttack && bAttacking)
	{
		// 공격 함수 실행, 시간을 설정하고 다음까지 대기...
	}
}

// Called every frame
void UNACombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

