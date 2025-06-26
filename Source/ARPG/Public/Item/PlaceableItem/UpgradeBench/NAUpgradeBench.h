// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemActor/NAPlaceableItemActor.h"
#include "NAUpgradeBench.generated.h"

UCLASS(Abstract)
class ARPG_API ANAUpgradeBench : public ANAPlaceableItemActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANAUpgradeBench(const FObjectInitializer& ObjectInitializer);
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

//======================================================================================================================
// Interactable Interface Implements
//======================================================================================================================
public:
	virtual bool CanInteract_Implementation() const override;
	virtual bool IsOnInteract_Implementation() const override;
	
	virtual bool TryInteract_Implementation(AActor* Interactor) override;

protected:
	virtual bool BeginInteract_Implementation(AActor* Interactor) override;
	virtual bool ExecuteInteract_Implementation(AActor* Interactor) override;
	virtual bool EndInteract_Implementation(AActor* Interactor) override;
};
