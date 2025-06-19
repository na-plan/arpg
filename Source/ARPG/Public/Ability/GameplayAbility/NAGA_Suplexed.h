// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "NAGA_Suplexed.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNAGA_Suplexed : public UGameplayAbility
{
	GENERATED_BODY()
	
	UNAGA_Suplexed();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);

	virtual void SetSuplexedMontage(UAnimMontage* InAnimMontage) { SuplexedMontage = InAnimMontage; }
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* SuplexedMontage;

	
	
};
