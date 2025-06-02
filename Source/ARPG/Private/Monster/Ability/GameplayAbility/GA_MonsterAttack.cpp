// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Ability/GameplayAbility/GA_MonsterAttack.h"

#include "Monster/Pawn/MonsterBase.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"


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
			UAbilitySystemComponent* MonsterASC =  MonsterBase->GetAbilitySystemComponent();
			//AttackCombo
			TArray<UAnimMontage*> AttackComboMontage = MonsterBase->GetAttackMontageCombo();
			uint8 MonsterAttackComboMontageNum = AttackComboMontage.Num();
			// 하나만 있을때
			UAnimMontage* MonsterAttackMontage = MonsterBase->GetAttackMontage();

			if (MonsterASC)
			{
				UAnimInstance* AnimInstance = MonsterASC->GetAvatarActor()->FindComponentByClass<USkeletalMeshComponent>()->GetAnimInstance();
				if (AnimInstance)
				{
					if (MonsterAttackComboMontageNum > 0)
					{
						float PlayingMontage = MonsterASC->PlayMontage(this, CurrentActivationInfo, AttackComboMontage[MonsterBase->GetComboState()], 1.0f);
						EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

						MonsterBase->SaveCombo();						
						if (MonsterBase->GetComboState() >= MonsterAttackComboMontageNum) { MonsterBase->ResetCombo(); }
					}
					else 
					{
						if (MonsterAttackMontage)
						{
							// 재생완료						
							float PlayingMontage = MonsterASC->PlayMontage(this, CurrentActivationInfo, MonsterAttackMontage, 1.0f);
							EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
						}

					}

				}

			}

		}
	}
}

void UGA_MonsterAttack::OnComplete()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);


}

void UGA_MonsterAttack::OnCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

	//사용하는 몽타주를 정지시킨뒤 Cancelled Montage를 재생시키면 튕겨내는 효과, 공격 도중에 날라가거나, 공격 중에 죽을때의 animation을 재생시킬수 있음
	//Attribute에서 상태이상 가져오고 그것에 따라서 montage를 가지고와서 재생 시키면 될거 같음


}
