// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/NAInteractableInterface.h"

#include "Interaction/NAInteractionComponent.h"
#include "Item/ItemDataStructs/NAItemBaseDataStructs.h"

// Add default functionality here for any INAInteractionInterface functions that are not pure virtual.

UNAInteractionComponent* INAInteractableInterface::TryGetInteractionComponent(AActor* InActor)
{
	UNAInteractionComponent* InteractionComp = nullptr;
	InteractionComp = InActor->FindComponentByClass<UNAInteractionComponent>();
	return InteractionComp;
}

bool INAInteractableInterface::CheckInteractableEdit(const FNAInteractableData& InteractableData) const
{
	if (!InteractableData.bModifiedFromBaseline)
	{
		const bool bIsModified = InteractableData.IsDifferentFromDefault();
		InteractableData.bModifiedFromBaseline = bIsModified;
	}
	return InteractableData.bModifiedFromBaseline;
}

const FNAInteractableData* INAInteractableInterface::TryGetInteractableData(const FNAItemBaseTableRow* InItemMetaData) const
{
	if (InItemMetaData)
	{
		return &InItemMetaData->InteractableData;
	}
	
	return nullptr;
}

void INAInteractableInterface::TrySetInteractableData(FNAItemBaseTableRow* InItemMetaData, const FNAInteractableData& NewInteractableData)
{
	if (InItemMetaData)
	{
		InItemMetaData->InteractableData = NewInteractableData;
	}
}

void INAInteractableInterface::MarkBaselineModified(const FNAInteractableData& OutInteractableData)
{
	OutInteractableData.bModifiedFromBaseline = true;
}