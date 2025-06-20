// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NALobbyHUD.generated.h"

class UNASessionListWidget;
class UNALobbyWidget;
/**
 * 
 */
UCLASS()
class ARPG_API ANALobbyHUD : public AHUD
{
	GENERATED_BODY()

public:
	ANALobbyHUD();

public:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = true))
	TSubclassOf<UUserWidget> LobbyWidgetClass;
	
	UPROPERTY()
	UNALobbyWidget* LobbyWidget;

	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = true))
	TSubclassOf<UUserWidget> SessionListWidgetClass;

	UPROPERTY()
	UNASessionListWidget* SessionListWidget;
};
