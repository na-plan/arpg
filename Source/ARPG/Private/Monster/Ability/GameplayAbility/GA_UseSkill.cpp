// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Ability/GameplayAbility/GA_UseSkill.h"

#include "Monster/Pawn/MonsterBase.h"
#include "AbilitySystemComponent.h"
#include "Skill/DataTable/SkillTableRow.h"


UGA_UseSkill::UGA_UseSkill()
{
	//Network 동기화 + client 반영
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UGA_UseSkill::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (HasAuthority(&ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}
	}

	
	if (UAbilitySystemComponent* ASC = ActorInfo->AvatarActor.Get()->FindComponentByClass<UAbilitySystemComponent>()) 
	{ 
		bool FindComponentSuc = true; 
		if (AMonsterBase* MonsterBase = CastChecked<AMonsterBase>(ActorInfo->AvatarActor.Get()))
		{
			if (UAnimMontage* SkillMontage = MonsterBase->GetSelectSkillMontage())
			{
				ASC->PlayMontage(this, CurrentActivationInfo, SkillMontage, 1.0f);

			}
		}

	}

}
