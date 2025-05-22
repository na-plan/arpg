// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayAbility/NAGA_Melee.h"

#include "AbilitySystemComponent.h"
#include "Ability/AttributeSet/NAAttributeSet.h"
#include "Combat/GameplayEffect/NAGE_UseActivePoint.h"

void UNAGA_Melee::OnMontageEnded(UAnimMontage* /*Montage*/, bool /*bInterrupted*/)
{
	// 델레게이션을 지우고, 효과를 종료함 (가정: 어빌리티가 인스턴싱 되는 경우)
	GetCurrentActorInfo()->AnimInstance->OnMontageEnded.RemoveAll(this);
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UNAGA_Melee::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                  const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}

		// todo: Melee Montage, Handling the effect from AnimNotify
		ActorInfo->AbilitySystemComponent->PlayMontage(this, ActivationInfo, nullptr, 1.f);

		// 효과는 몽타주가 끝나는 시점에 종료 판정이 남
		ActorInfo->AnimInstance->OnMontageEnded.AddUniqueDynamic(this, &UNAGA_Melee::OnMontageEnded);
	}
}

bool UNAGA_Melee::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags)
{
	bool bResult = true;

	bResult &= ActorInfo->AbilitySystemComponent->GetCurrentMontage() == nullptr;
	bResult &= Cast<UNAAttributeSet>(ActorInfo->AbilitySystemComponent->GetAttributeSet(UNAAttributeSet::StaticClass()))->GetHealth() > 0;
	bResult &= Cast<UNAAttributeSet>(ActorInfo->AbilitySystemComponent->GetAttributeSet(UNAAttributeSet::StaticClass()))->GetAP() > 0;
	
	{
		// AP 포인트 감소
		const FGameplayEffectContextHandle ContextHandle = ActorInfo->AbilitySystemComponent->MakeEffectContext();
		const FGameplayEffectSpecHandle SpecHandle = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(UNAGE_UseActivePoint::StaticClass(), 1.f, ContextHandle);
		const FActiveGameplayEffectHandle EffectHandle = ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		bResult &= EffectHandle.WasSuccessfullyApplied();
	}

	return bResult;
}
