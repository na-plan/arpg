// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_UseSkill.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UGA_UseSkill : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	//UPROPERTY(EditDefaultsOnly)
	//FSkillDataRow* SkillData;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);


	
};
