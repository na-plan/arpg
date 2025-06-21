// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "NAGA_FireGun.generated.h"

class UNAAT_WaitRotation;
/**
 * 
 */
UCLASS()
class ARPG_API UNAGA_FireGun : public UGameplayAbility
{
	GENERATED_BODY()

	UNAGA_FireGun();

	static bool ConsumeAmmo(UAbilitySystemComponent* InAbilitySystemComponent, const TSubclassOf<UGameplayEffect>& InAmmoType);
	
	UFUNCTION()
	void OnRotationCompleted();

	void Fire( APawn* ControlledPawn );
	
	UPROPERTY()
	UNAAT_WaitRotation* WaitRotationTask;

protected:
	UFUNCTION()
	void OnMontageEnded( UAnimMontage* AnimMontage, bool bInterrupted );

	void CancelAbilityProxy( FGameplayTag GameplayTag, int Count );
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
