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

protected:
	virtual void OnPossess(APawn* InPawn) override;

	virtual void AcknowledgePossession(class APawn* P) override;
	
	virtual void OnUnPossess() override;
};
