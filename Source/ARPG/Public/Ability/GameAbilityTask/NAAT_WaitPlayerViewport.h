// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "NAAT_WaitPlayerViewport.generated.h"

class ANAPlayerController;
/**
 * 
 */
UCLASS()
class ARPG_API UNAAT_WaitPlayerViewport : public UAbilityTask
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UNAAT_WaitPlayerViewport* WaitPlayerViewport( UGameplayAbility* OwningAbility, FName TaskInstanceName, UAbilitySystemComponent* CasterAbilitySystemComponent, AActor* InTargetShouldConcentrate );
	
	virtual void TickTask(float DeltaTime) override;

protected:
	UPROPERTY()
	UAbilitySystemComponent* CasterAbilitySystemComponent;

	UPROPERTY()
	AActor* TargetShouldConcentrate;
};
