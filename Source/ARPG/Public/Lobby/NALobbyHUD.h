// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NALobbyHUD.generated.h"

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

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	UPROPERTY(meta = (BindWidget))
	UNALobbyWidget* LobbyWidget;

	UPROPERTY()
	TSubclassOf<UNALobbyWidget> LobbyWidgetBP;
};
