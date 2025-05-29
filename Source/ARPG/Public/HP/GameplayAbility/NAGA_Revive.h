// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
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
	
	FTimerHandle ReviveTimer;

	FDelegateHandle ReviveCancelHandle;
	
protected:
	void OnReviveSucceeded();

	void OnHitWhileRevive( const FOnAttributeChangeData& OnAttributeChangeData );
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) override;
	
};
