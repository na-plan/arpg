// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "NAGA_Grab.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNAGA_Grab : public UGameplayAbility
{
	GENERATED_BODY()
	
	UNAGA_Grab();
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);

	virtual void SetSuplexMontage(UAnimMontage* InAnimMontage) { SuplexingMontage = InAnimMontage; }
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* SuplexingMontage;
};
