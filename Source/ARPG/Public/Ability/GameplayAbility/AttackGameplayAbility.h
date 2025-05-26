// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AttackGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UAttackGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		bool bReplicateCancelAbility) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		bool bReplicateEndAbility, bool bWasCancelled) override;

<<<<<<< HEAD
=======
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TObjectPtr<UAnimMontage> AttackAnimMontage;
>>>>>>> d34d6c1 (11)

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TObjectPtr<UAnimMontage> AttackAnimMontage;

	bool IsTargetInRange(AActor* TargetActor, float Range);


	
};
