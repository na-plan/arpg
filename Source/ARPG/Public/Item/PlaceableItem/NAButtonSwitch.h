// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemActor/NAPlaceableItemActor.h"
#include "NAButtonSwitch.generated.h"

DECLARE_DYNAMIC_DELEGATE(FOnActivateTargetActor);

UCLASS()
class ARPG_API ANAButtonSwitch : public ANAPlaceableItemActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANAButtonSwitch(const FObjectInitializer& ObjectInitializer);
		
	virtual void PostRegisterAllComponents() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents() override;

	//======================================================================================================================
	// Interactable Interface Implements
	//======================================================================================================================
public:
	virtual void BeginInteract_Implementation(AActor* Interactor) override;
	virtual void EndInteract_Implementation(AActor* Interactor) override;
	virtual void ExecuteInteract_Implementation(AActor* Interactor) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	ANAPlaceableItemActor* TargetToActive;

	UPROPERTY()
	bool bIsActive = false;

	// Delegate
	FOnActivateTargetActor OnActivateTarget;
};
