// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NAPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API ANAPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ANAPlayerController();
	
protected:
	virtual void BeginPlay() override;
	
	virtual void OnPossess(APawn* InPawn) override;

	virtual void AcknowledgePossession(APawn* P) override;

	virtual void OnRep_PlayerState() override;
	
	virtual void OnUnPossess() override;
};
