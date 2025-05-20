// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Ability/GameplayAbility/GA_MonsterAttack.h"

#include "Monster/Pawn/MonsterBase.h"
#include "AbilitySystemComponent.h"


void UGA_MonsterAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		if (AMonsterBase* MonsterBase = CastChecked<AMonsterBase>(ActorInfo->AvatarActor.Get()))
		{
			UAnimMontage* MonsterAttackMontage = MonsterBase->GetAttackMontage();
			UAbilitySystemComponent* MonsterASC =  MonsterBase->GetAbilitySystemComponent();

			if (MonsterAttackMontage && MonsterASC)
			{
				float MontageDuration = MonsterASC->PlayMontage(this, GetCurrentActivationInfo(), MonsterAttackMontage, 1.0f);
			}
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		}
	}
}
