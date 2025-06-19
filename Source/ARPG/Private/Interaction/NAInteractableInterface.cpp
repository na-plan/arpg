// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/NAInteractableInterface.h"

#include "Interaction/NAInteractionComponent.h"

// Add default functionality here for any INAInteractionInterface functions that are not pure virtual.

UNAInteractionComponent* INAInteractableInterface::GetInteractionComponent(AActor* Interactor) const
{
	UNAInteractionComponent* InteractionComp = nullptr;
	InteractionComp = Interactor->FindComponentByClass<UNAInteractionComponent>();
	return InteractionComp;
}
