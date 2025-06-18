// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NALobbyGameMode.generated.h"

class UNALobbyWidget;
/**
 * 
 */
UCLASS()
class ARPG_API ANALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANALobbyGameMode();

public:
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = true))
	TSubclassOf<UUserWidget> LobbyWidgetClass;
	
	UPROPERTY()
	UNALobbyWidget* LobbyWidget;
};
