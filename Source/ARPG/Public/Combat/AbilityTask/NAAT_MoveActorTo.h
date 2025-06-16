// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "NAAT_MoveActorTo.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNAAT_MoveActorTo : public UAbilityTask
{
	GENERATED_BODY()

	float Mass;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Ability|Tasks", meta = (AllowPrivateAccess = "true") )
	TWeakObjectPtr<AActor> OriginActor;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Ability|Tasks", meta = (AllowPrivateAccess = "true") )
	TWeakObjectPtr<AActor> TargetActor;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Ability|Tasks", meta = (AllowPrivateAccess = "true") )
	TWeakObjectPtr<UPrimitiveComponent> TargetBoundComponent;
	
	ECollisionResponse PreviousResponse;
	
public:
	UNAAT_MoveActorTo();
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UNAAT_MoveActorTo* MoveActorTo( UGameplayAbility* OwningAbility, FName TaskInstanceName, AActor* Origin, AActor* Target, UPrimitiveComponent* InTargetBoundComponent );

	virtual void Activate() override;
	
	virtual void TickTask(float DeltaTime) override;

	virtual void OnDestroy(bool bInOwnerFinished) override;
};
