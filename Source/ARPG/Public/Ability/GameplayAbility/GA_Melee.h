// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Melee.generated.h"


class UAbilityTask_PlayMontageAndWait;
/**
 * 
 */
UCLASS()
class ARPG_API UGA_Melee : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 9b538d7 (44)
	UAnimMontage* MeleeAttackMontageToPlay;

	//	Damage UGameplayEffect를 추가해주세요
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageGameplayEffect;
<<<<<<< HEAD
=======
	UAnimMontage* MontageToPlay;
>>>>>>> 6640ccb (44)
=======
>>>>>>> 9b538d7 (44)

	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* MontageTask;
	
	UGA_Melee();

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 9b538d7 (44)
	// 실행 가능한지 확인용
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;

	/** Actually activate ability, do not call this directly. We'll call it from APAHeroCharacter::ActivateAbilitiesWithTags(). */
<<<<<<< HEAD
=======
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;

>>>>>>> 6640ccb (44)
=======
>>>>>>> 9b538d7 (44)
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);

protected:
	//FGameplayTag 로 공격 모션 다르게 하는거	-> 무기에 FGameplayTag 달아서 바꾸거나 하면 되려나?
	UFUNCTION()
	void OnCancelled(FGameplayTag EventTag, FGameplayEventData EventData);

	UFUNCTION()
	void OnCompleted(FGameplayTag EventTag, FGameplayEventData EventData);

	UFUNCTION()
	void EventReceived(FGameplayTag EventTag, FGameplayEventData EventData);
	
};
