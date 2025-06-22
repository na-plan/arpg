// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayAbility/NAGA_Melee.h"

#include "AbilitySystemComponent.h"
#include "Ability/AttributeSet/NAAttributeSet.h"
#include "Combat/ActorComponent/NACombatComponent.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "Combat/GameplayEffect/NAGE_UseActivePoint.h"

UNAGA_Melee::UNAGA_Melee(): bUseGrabMontage( false )
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UNAGA_Melee::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 델레게이션을 지우고, 효과를 종료함 (가정: 어빌리티가 인스턴싱 되는 경우)
	const FGameplayAbilityActivationInfo Info = GetCurrentActivationInfo();

	if ( const UNAMontageCombatComponent* CombatComponent = GetCurrentActorInfo()->AvatarActor->GetComponentByClass<UNAMontageCombatComponent>() )
	{
		if ( CombatComponent->GetMontage() == Montage )
		{
			// 추적하던 몽타주를 해제
			GetCurrentActorInfo()->AbilitySystemComponent->AbilityActorInfo->GetAnimInstance()->OnMontageEnded.RemoveAll(this);
			
			EndAbility( GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, bInterrupted );
			return;
		}
	}

	EndAbility( GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true );
}

void UNAGA_Melee::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                  const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if ( HasAuthorityOrPredictionKey( ActorInfo, &ActivationInfo ) )
	{
		if ( !CommitAbility(Handle, ActorInfo, ActivationInfo) )
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}

		if ( UAnimInstance* AnimInstance = ActorInfo->AbilitySystemComponent->AbilityActorInfo->GetAnimInstance() )
		{
			// 효과는 몽타주가 끝나는 시점에 종료 판정이 남
			AnimInstance->OnMontageEnded.AddUniqueDynamic( this, &UNAGA_Melee::OnMontageEnded );
		}

		if ( HasAuthority( &ActivationInfo ) )
		{
			// AP 포인트 감소
			const FGameplayEffectContextHandle ContextHandle = ActorInfo->AbilitySystemComponent->MakeEffectContext();
			const FGameplayEffectSpecHandle SpecHandle = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(UNAGE_UseActivePoint::StaticClass(), 1.f, ContextHandle);
			const FActiveGameplayEffectHandle EffectHandle = ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			check( EffectHandle.WasSuccessfullyApplied() );
		}
	}

	if (UNAMontageCombatComponent* CombatComponent = ActorInfo->AvatarActor->GetComponentByClass<UNAMontageCombatComponent>())
	{
		// grabmontage 이면 montage 바꾸도록 하기
		if (bUseGrabMontage)
		{
			ActorInfo->AbilitySystemComponent->PlayMontage
			(
				this,
				ActivationInfo,
				CombatComponent->GetGrabMontage(),
				CombatComponent->GetMontagePlayRate()
			);
		}
		else
		{
			ActorInfo->AbilitySystemComponent->PlayMontage
			(
				this,
				ActivationInfo,
				CombatComponent->GetMontage(),
				CombatComponent->GetMontagePlayRate()
			);
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
	return bResult;
}

void UNAGA_Melee::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	// 추적하던 몽타주를 해제
	if ( HasAuthorityOrPredictionKey( ActorInfo, &ActivationInfo ) )
	{
		if ( HasAuthority( &ActivationInfo ) && ActorInfo )
		{
			if ( const UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get())
			{
				if (UAnimInstance* AnimInstance = AbilitySystemComponent->AbilityActorInfo->GetAnimInstance())
				{
					AnimInstance->OnMontageEnded.RemoveAll(this);	
				}
			}
		}
	}
}
