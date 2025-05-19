// Fill out your copyright notice in the Description page of Project Settings.

#include "Ability/GameplayAbility/GA_Melee.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "NACharacter.h"
#include "Monster/Pawn/MonsterBase.h"

UGA_Melee::UGA_Melee()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
}

bool UGA_Melee::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	//Owner의 avator 를 가지고 오고
	ANACharacter* OwnerCharacter = CastChecked<ANACharacter>(ActorInfo->AvatarActor.Get());
	if (!OwnerCharacter) { return false; }

	
	// Montage 재생 확인 
	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		return !AnimInstance->Montage_IsPlaying(MontageToPlay);
	}

	//AnimInstance 없음 추가 ㄱㄱ

	return false;
}

void UGA_Melee::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (Character)
	{
	}
	AMonsterBase* OwnerMonster = Cast<AMonsterBase>(ActorInfo->AvatarActor.Get());
	if (OwnerMonster)
	{

	}

}

void UGA_Melee::OnCancelled(FGameplayTag EventTag, FGameplayEventData EventData)
{
}

void UGA_Melee::OnCompleted(FGameplayTag EventTag, FGameplayEventData EventData)
{
}

void UGA_Melee::EventReceived(FGameplayTag EventTag, FGameplayEventData EventData)
{
}
