// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Ability/GameplayAbility/GA_UseSkill.h"

#include "Monster/Pawn/MonsterBase.h"
#include "AbilitySystemComponent.h"
#include "Skill/DataTable/SkillTableRow.h"


void UGA_UseSkill::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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
			//UAnimMontage* MonsterSpawnMontage = MonsterBase->GetSpawnMontage();
			FDataTableRowHandle OwnSkillData = MonsterBase->GetSkillData();
			FOwnSkillTable* Data = OwnSkillData.GetRow<FOwnSkillTable>(TEXT("MonsterSkillData"));

			uint8 MonsterOwnskillnum = Data->OwnSkillArray.Num();
			UAbilitySystemComponent* MonsterASC = MonsterBase->GetAbilitySystemComponent();

			if (MonsterASC && MonsterOwnskillnum > 0)
			{
				//Num 가져오는 과정
				FSkillTableRow CurrentData = Data->OwnSkillArray[MonsterOwnskillnum];

				//ability에 parry를 던지고

			}

		}
	}
}
