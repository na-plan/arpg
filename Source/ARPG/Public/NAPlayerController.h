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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UNAInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UNAInventoryWidget> InventoryWidget = nullptr;

public:
	ANAPlayerController();
	
protected:
	virtual void BeginPlay() override;
	
	virtual void OnPossess(APawn* InPawn) override;

	virtual void AcknowledgePossession(APawn* P) override;
	
	virtual void OnUnPossess() override;
};
