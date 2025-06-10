// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_ParryStun.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UGA_ParryStun : public UGameplayAbility
{
	GENERATED_BODY()
	
	//Player 가 공격을 실행 하고 monster한테 전달을 해주는 방식으로 하기로함
	//그러면? player가 공격하는 타이밍이 공격하는 몽타주의 특정 부분일때 
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);

	
	
	
};
