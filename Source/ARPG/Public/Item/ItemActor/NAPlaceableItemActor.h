#pragma once

#include "Item/ItemActor/NAItemActor.h"
#include "NAPlaceableItemActor.generated.h"

UCLASS()
class ARPG_API ANAPlaceableItemActor : public ANAItemActor
{
	GENERATED_BODY()
	
	// Macro Hooks
	IMPLEMENT_MACROHOOK_GetInitInteractableDataParams(ENAInteractableType::Placeable, 0.f, 1);

public:
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
	virtual void BeginPlay() override;
};