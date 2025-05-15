// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPG/Public/NAPlayerController.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "NAPlayerState.h"

void ANAPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void ANAPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);
	
	if (const TScriptInterface<IAbilitySystemInterface> Interface(P); Interface)
	{
		Interface->GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState<ANAPlayerState>(), P);
	}
}

void ANAPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
}
