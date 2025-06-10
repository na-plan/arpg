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

//void UGA_UseSkill::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
//{
//
//}
//
//void UGA_UseSkill::OnMontageStarted(UAnimMontage* Montage)
//{
//	//여기에서 스킬 선택을 하면 어떨까? -> 이미 선택된걸 가지고 와서 start하기 때문에 안됌
//	FGameplayAbilityActorInfo OwnerActorInfo = GetActorInfo();
//	AMonsterBase* MonsterBase = CastChecked<AMonsterBase>(OwnerActorInfo.AvatarActor.Get());
//	AMonsterAIController* MonsterAI = CastChecked<AMonsterAIController>(MonsterBase->GetController());
//	MonsterAI->UsedSkill();
//}

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
				//공격 콤보 리셋
				MonsterBase->ResetCombo();
				//UAnimInstance* AnimInstance = ASC->AbilityActorInfo->GetAnimInstance();
				//AnimInstance->OnMontageEnded.AddUniqueDynamic(this, &UGA_UseSkill::OnMontageEnded);
				//AnimInstance->OnMontageStarted.AddUniqueDynamic(this, &UGA_UseSkill::OnMontageStarted);

			}
		}

	}

}

