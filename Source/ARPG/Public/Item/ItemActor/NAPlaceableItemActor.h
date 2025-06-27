#pragma once

#include "Item/ItemActor/NAItemActor.h"
#include "NAPlaceableItemActor.generated.h"

UCLASS(Abstract)
class ARPG_API ANAPlaceableItemActor : public ANAItemActor
{
	GENERATED_BODY()

public:
	ANAPlaceableItemActor(const FObjectInitializer& ObjectInitializer);

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
};