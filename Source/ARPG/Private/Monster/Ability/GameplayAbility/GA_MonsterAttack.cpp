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
			UAnimMontage* MonsterAttackMontage = MonsterBase->GetAttackMontage();
			UAbilitySystemComponent* MonsterASC =  MonsterBase->GetAbilitySystemComponent();

			// 들어오는거 확인 모두 보유중
			if (MonsterAttackMontage && MonsterASC)
			{
				// 재생완료
				UAnimInstance* AnimInstance = MonsterASC->GetAvatarActor()->FindComponentByClass<USkeletalMeshComponent>()->GetAnimInstance();
				if (AnimInstance)
				{
					float PlayingMontage = MonsterASC->PlayMontage(this, CurrentActivationInfo, MonsterAttackMontage, 1.0f);
				}



				//추가 수정		AbilityTask를 만든 뒤에 사용해주세요
				//UAbilityTask_PlayMontageAndWait* PlayMontageTask= UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("NONE"), MonsterAttackMontage, 1.0f);
				//PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_MonsterAttack::OnComplete);
				//// 플레이어가 특정 행동을 강제하거나, 상태 이상(스턴 등)으로 인해 능력이 취소되도록 하는것도 좋을거 같음
				//PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_MonsterAttack::OnCancelled);
				//PlayMontageTask->ReadyForActivation();


				EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
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
