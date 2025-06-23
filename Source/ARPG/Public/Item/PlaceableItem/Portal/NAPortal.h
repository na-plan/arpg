// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemActor/NAPlaceableItemActor.h"
#include "NAPortal.generated.h"

UCLASS()
class ARPG_API ANAPortal : public ANAPlaceableItemActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANAPortal(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void PostInitProperties() override;
	virtual void PostRegisterAllComponents() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents() override;
	
protected:
	virtual void BeginPlay() override;

	//======================================================================================================================
	// Interactable Interface Implements
	//======================================================================================================================
protected:
	virtual bool BeginInteract_Implementation(AActor* Interactor) override;
	virtual bool ExecuteInteract_Implementation(AActor* Interactor) override;
	virtual bool EndInteract_Implementation(AActor* Interactor) override;
	
	virtual void SetInteractionPhysicsEnabled(const bool bEnabled) override;
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UFUNCTION()
	void TeleportToDest(AActor* InterActor);

protected:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	AActor* DestActor;
};
