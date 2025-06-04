// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPG/Public/NAPlayerController.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "NACharacter.h"
#include "NAPlayerState.h"
#include "ARPG/ARPG.h"


ANAPlayerController::ANAPlayerController()
{
}

void ANAPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ANAPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	// if (InteractionComponent)
	// {
	// 	AttachToPawn(InPawn);
	// }
}

void ANAPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);
}

void ANAPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}

void ANAPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	// if (InteractionComponent)
	// {
	// 	DetachFromPawn();
	// }
}
