// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "NAAT_WaitRotation.generated.h"

DECLARE_DYNAMIC_DELEGATE( FOnRotationCompleted );

/**
 * 
 */
UCLASS()
class ARPG_API UNAAT_WaitRotation : public UAbilityTask
{
	GENERATED_BODY()
	
	FQuat TargetRotation;

	FRotator TargetRotator;

	FVector TargetForward;

	void Rotate();
	
public:
	
	UNAAT_WaitRotation();
	
	virtual void Activate() override;

	void ForceRotate() const;
	
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
	virtual void TickTask(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UNAAT_WaitRotation* WaitRotation( UGameplayAbility* OwningAbility, FName TaskInstanceName, const FQuat& TargetRotation );
	
	FOnRotationCompleted OnRotationCompleted;
	
};
