// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/ActorComponent/NAMontageCombatComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UNAMontageCombatComponent::UNAMontageCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UNAMontageCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

bool UNAMontageCombatComponent::IsAbleToAttack()
{
	bool bResult = true;

	// 캐릭터가 몽타주를 실행중이지 않을 경우
	// todo: 캐릭터가 공격중이지 않은 경우로?
	if (const TScriptInterface<IAbilitySystemInterface>& Interface = GetAttacker())
	{
		bResult &= Interface->GetAbilitySystemComponent()->GetCurrentMontage() == nullptr;
	}
	
	return bResult;
}

void UNAMontageCombatComponent::OnAttack_Implementation()
{
	// note: 몽타주 재생 및 전파는 GAS의 Ability를 통해 수행!
	// Ability에서 Montage Combat Component가 들고 있는 데이터를 가지고서 수행해야 작동이 맞음
	
	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttackSound, GetOwner()->GetActorLocation());

		// 서버 구동일 경우에는 사운드 재생을 전파
		if (GetNetMode() != NM_Standalone)
		{
			Multi_PlaySound();
		}
	}
}

void UNAMontageCombatComponent::Multi_PlaySound_Implementation() const
{
	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttackSound, GetOwner()->GetActorLocation());
	}
}

float UNAMontageCombatComponent::GetNextAttackTime()
{
	return AttackMontage->GetPlayLength() * MontagePlayRate;
}

// Called every frame
void UNAMontageCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

