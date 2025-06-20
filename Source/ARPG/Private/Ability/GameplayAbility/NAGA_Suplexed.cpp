// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayAbility/NAGA_Suplexed.h"
#include "AbilitySystemComponent.h"
#include "Monster/Pawn/MonsterBase.h"

UNAGA_Suplexed::UNAGA_Suplexed()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UNAGA_Suplexed::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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
		UAnimMontage* MonsterSuplexedMontage = MonsterBase->GetSuplexedMontage();
		UAbilitySystemComponent* MonsterASC = MonsterBase->GetAbilitySystemComponent();
		// 들어오는거 확인 모두 보유중
		if (MonsterSuplexedMontage && MonsterASC)
		{
			UAnimInstance* AnimInstance = MonsterASC->GetAvatarActor()->FindComponentByClass<USkeletalMeshComponent>()->GetAnimInstance();
			AMonsterAIController* MonsterAIController = CastChecked<AMonsterAIController>(MonsterBase->GetController());
			if (MonsterSuplexedMontage)
			{
				if (AnimInstance && MonsterAIController)
				{
					MonsterASC->AbilityActorInfo->GetAnimInstance()->Montage_Stop(0.2f);
					float PlayingMontage = MonsterASC->PlayMontage(this, CurrentActivationInfo, MonsterSuplexedMontage, 1.0f);
				}
			}
		}
	}
}
