// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "NAGA_KineticGrab.generated.h"

class UNAAT_MoveActorTo;
class UNAAT_ConsumeKineticGrabAP;
/**
 * 
 */
UCLASS()
class ARPG_API UNAGA_KineticGrab : public UGameplayAbility
{
	GENERATED_BODY()

	UNAGA_KineticGrab();

	ECollisionResponse PreviousResponse;
	
	UPROPERTY()
	UNAAT_ConsumeKineticGrabAP* APConsumeTask;

	UPROPERTY()
	UNAAT_MoveActorTo* MoveActorToTask;
	
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

	UFUNCTION()
	void OnAPDepleted();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	
public:
	static FVector EvaluateActorPosition( const AActor* OriginActor, const UPrimitiveComponent* TargetBoundComponent,
	                                      const FVector& OriginForwardVector, float Distance );

	static FVector EvaluateActorPosition( const AActor* OriginActor, const FVector& OriginForwardVector, float MinimumDistance );

	static FVector GetMinimumDistance( const AActor* OriginActor, const UPrimitiveComponent* TargetBoundComponent, const FVector& ForwardVector );
	
	UFUNCTION()
	void Throw();
};
