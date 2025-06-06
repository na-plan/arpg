// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "NAGA_FireGun.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNAGA_FireGun : public UGameplayAbility
{
	GENERATED_BODY()

	UNAGA_FireGun();

	static bool ConsumeAmmo(UAbilitySystemComponent* InAbilitySystemComponent);

protected:
	UFUNCTION()
	void OnMontageEnded( UAnimMontage* AnimMontage, bool bInterrupted );
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) override;
};
