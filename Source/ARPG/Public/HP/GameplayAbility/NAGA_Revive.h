// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ARPG/ARPG.h"
#include "HP/ActorComponent/NAVitalCheckComponent.h"
#include "NAGA_Revive.generated.h"

class UNAAT_WaitPlayerViewport;
class ANACharacter;
/**
 * 
 */
UCLASS()
class ARPG_API UNAGA_Revive : public UGameplayAbility
{
	GENERATED_BODY()

	UPROPERTY( VisibleAnywhere )
	TWeakObjectPtr<ANACharacter> RevivingTarget;

	UPROPERTY( VisibleAnywhere )
	UNAAT_WaitPlayerViewport* ViewportCheckTask;

	FTimerHandle ReviveTimerHandle;

	FDelegateHandle RevivingTargetHandle;
	
	FDelegateHandle ReviveCancelHandle;
	
protected:
	void OnReviveSucceeded();

	void OnHitWhileRevive( const FOnAttributeChangeData& OnAttributeChangeData );
	
	void CheckKnockDownDead( UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle );
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) override;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
};
