// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayAbility/NAGA_Suplex.h"
#include "NACharacter.h"
#include "AbilitySystemComponent.h"
#include "Combat/ActorComponent/NACombatComponent.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"

UNAGA_Suplex::UNAGA_Suplex()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

}

void UNAGA_Suplex::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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
		if (UNAMontageCombatComponent* CombatComponent = ActorInfo->AvatarActor->GetComponentByClass<UNAMontageCombatComponent>())
		{
			ActorInfo->AbilitySystemComponent->PlayMontage
			(
				this,
				ActivationInfo,
				CombatComponent->GetGrabMontage(),
				CombatComponent->GetMontagePlayRate()
			);
		}

	}
}
