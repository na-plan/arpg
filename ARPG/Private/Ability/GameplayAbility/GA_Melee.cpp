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
		return !AnimInstance->Montage_IsPlaying(MeleeAttackMontageToPlay);
	}


	return false;
}

void UGA_Melee::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
	UAnimMontage* MontageToPlay = MeleeAttackMontageToPlay;
	
	// RequestGameplayTag 확인후 몽타주 재생 (아직 태그를 안만들어서 주석 처리함
	// 근접 공격 상태(상대 몬스터 그로기 + 상호작용 가능할때 활성화 ㄱㄱ)
	//if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Melee"))) &&
	//	!GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Melee.Removal"))))
	//{
	//	MontageToPlay = MeleeAttackMontageToPlay;
	//}
	
	// 델리게이트 바인딩 Task 만들고 각 이벤트를 콜백해서 바인딩 해야됌 
	//UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MontageToPlay, 1.0f, NAME_None, false, 1.0f);
	//Task->OnBlendOut.AddDynamic(this, &UGA_Melee::OnCompleted);
	//Task->OnCompleted.AddDynamic(this, &UGA_Melee::OnCompleted);
	//Task->OnInterrupted.AddDynamic(this, &UGA_Melee::OnCancelled);
	//Task->OnCancelled.AddDynamic(this, &UGA_Melee::OnCancelled);
	//Task->EventReceived.AddDynamic(this, &UGA_Melee::EventReceived);
	//// ReadyForActivation() is how you activate the AbilityTask in C++. Blueprint has magic from K2Node_LatentGameplayTaskCall that will automatically call ReadyForActivation().
	//Task->ReadyForActivation();
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
