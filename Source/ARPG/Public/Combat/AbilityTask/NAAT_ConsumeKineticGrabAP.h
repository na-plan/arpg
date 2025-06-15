// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "NAAT_ConsumeKineticGrabAP.generated.h"

DECLARE_MULTICAST_DELEGATE( FOnAPDepleted );

/**
 * 
 */
UCLASS()
class ARPG_API UNAAT_ConsumeKineticGrabAP : public UAbilityTask
{
	GENERATED_BODY()

	virtual void TickTask(float DeltaTime) override;

public:
	FOnAPDepleted OnAPDepleted;

	UNAAT_ConsumeKineticGrabAP();
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UNAAT_ConsumeKineticGrabAP* WaitAPDepleted( UGameplayAbility* OwningAbility, FName TaskInstanceName );
};
