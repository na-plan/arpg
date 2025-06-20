// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Ability/GameplayAbility/NAGA_Death.h"

#include "Monster/Pawn/MonsterBase.h"
#include "AbilitySystemComponent.h"
#include "Skill/DataTable/SkillTableRow.h"


UNAGA_Death::UNAGA_Death()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UNAGA_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (HasAuthority(&ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}
	}

	if (AMonsterBase* MonsterBase = CastChecked<AMonsterBase>(ActorInfo->AvatarActor.Get()))
	{
		UAnimMontage* MonsterDeathMontage = MonsterBase->GetDeathMontage();
		UAnimMontage* MonsterSuplexedMontage = MonsterBase->GetSuplexedMontage();
		UAbilitySystemComponent* MonsterASC = MonsterBase->GetAbilitySystemComponent();
		//UAbilitySystemComponent* MonsterASC = ActorInfo->AvatarActor.Get()->FindComponentByClass<UAbilitySystemComponent>();
		// 들어오는거 확인 모두 보유중
		if ((MonsterSuplexedMontage || MonsterDeathMontage) && MonsterASC)
		{
			UAnimInstance* AnimInstance = MonsterASC->GetAvatarActor()->FindComponentByClass<USkeletalMeshComponent>()->GetAnimInstance();
			AMonsterAIController* MonsterAIController = CastChecked<AMonsterAIController>(MonsterBase->GetController());
			if (MonsterSuplexedMontage)
			{				
				if (MonsterASC->GetCurrentMontage() != MonsterSuplexedMontage)
				{
					// 얘가 NULL 로 들어옴
					UAnimMontage* CheckMontage = MonsterASC->GetCurrentMontage();
					if (AnimInstance && MonsterAIController)
					{
						MonsterASC->AbilityActorInfo->GetAnimInstance()->Montage_Stop(0.2f);
						float PlayingMontage = MonsterASC->PlayMontage(this, CurrentActivationInfo, MonsterDeathMontage, 1.0f);
					}
				}
				else
				{
					bool CheckSupled = true;

				}
			}
			else
			{
				if (AnimInstance && MonsterAIController)
				{
					MonsterASC->AbilityActorInfo->GetAnimInstance()->Montage_Stop(0.2f);
					float PlayingMontage = MonsterASC->PlayMontage(this, CurrentActivationInfo, MonsterDeathMontage, 1.0f);
				}
			}


		}
		// Death Montage 가 없을 경우 바로 ai작동
		else
		{
			AMonsterAIController* MonsterAIController = CastChecked<AMonsterAIController>(MonsterBase->GetController());
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			MonsterAIController->GetOwner()->Destroy();
		}
	}
}
