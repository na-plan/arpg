// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_MonsterAttack.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UGA_MonsterAttack : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* MeleeAttackMontageToPlay;

	//	Damage UGameplayEffect를 추가해주세요
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageGameplayEffect;

	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);

<<<<<<< HEAD



	//OnCompleteCallback
	UFUNCTION()
	virtual void OnComplete();
	UFUNCTION()
	virtual void OnCancelled();
=======
>>>>>>> 9b538d7 (44)
	
};
