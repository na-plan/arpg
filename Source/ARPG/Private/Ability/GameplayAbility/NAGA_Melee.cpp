// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayAbility/NAGA_Melee.h"

#include "AbilitySystemComponent.h"
#include "Ability/AttributeSet/NAAttributeSet.h"
#include "Combat/ActorComponent/NACombatComponent.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "Combat/GameplayEffect/NAGE_UseActivePoint.h"

UNAGA_Melee::UNAGA_Melee()
{	
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UNAGA_Melee::OnMontageEnded(UAnimMontage* /*Montage*/, bool /*bInterrupted*/)
{
	// 델레게이션을 지우고, 효과를 종료함 (가정: 어빌리티가 인스턴싱 되는 경우)
	const FGameplayAbilityActivationInfo Info = GetCurrentActivationInfo();

	// 추적하던 몽타주를 해제
	if ( HasAuthority( &Info ) )
	{
		GetCurrentActorInfo()->AbilitySystemComponent->AbilityActorInfo->GetAnimInstance()->OnMontageEnded.RemoveAll(this);
	}
	
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UNAGA_Melee::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                  const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 실행 가능 여부 확인은 서버로
	if ( HasAuthority(&ActivationInfo) )
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}
		
		if (UNAMontageCombatComponent* CombatComponent = ActorInfo->AvatarActor->GetComponentByClass<UNAMontageCombatComponent>())
		{
			ActorInfo->AbilitySystemComponent->PlayMontage
			(
				this,
				ActivationInfo,
				CombatComponent->GetMontage(),
				CombatComponent->GetMontagePlayRate()
			);

			if (UAnimInstance* AnimInstance = ActorInfo->AbilitySystemComponent->AbilityActorInfo->GetAnimInstance())
			{
				// 효과는 몽타주가 끝나는 시점에 종료 판정이 남
				AnimInstance->OnMontageEnded.AddUniqueDynamic(this, &UNAGA_Melee::OnMontageEnded);	
			}
			else
			{
				// 무슨 이유인지는 몰라도 AnimInstance가 없는걸로 나올때가 있음
				check( false );
			}
		}
		else
		{
			// Combat Component가 없음
			check( false );
		}
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

void UNAGA_Melee::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	// 추적하던 몽타주를 해제
	if ( HasAuthority(&ActivationInfo) )
	{
		if ( const UAbilitySystemComponent* AbilitySystemComponent = GetCurrentActorInfo()->AbilitySystemComponent.Get())
		{
			if (UAnimInstance* AnimInstance = AbilitySystemComponent->AbilityActorInfo->GetAnimInstance())
			{
				AnimInstance->OnMontageEnded.RemoveAll(this);	
			}
			else
			{
				// 무슨 이유인지는 몰라도 AnimInstance가 없는걸로 나올때가 있음
				check( false );
			}	
		}
	}
	
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}
