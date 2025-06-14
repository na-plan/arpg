// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/NALobbyGameMode.h"

#include "NAGameStateBase.h"
#include "NAPlayerController.h"
#include "NAPlayerState.h"
#include "Kismet/GameplayStatics.h"

ANALobbyGameMode::ANALobbyGameMode()
{
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
	PlayerStateClass = ANAPlayerState::StaticClass();
	GameStateClass = ANAGameStateBase::StaticClass();
	PlayerControllerClass = ANAPlayerController::StaticClass();
	
	
}

void ANALobbyGameMode::ChangeLevel(UObject* WorldContext, FString LevelName)
{
	UGameplayStatics::OpenLevel(WorldContext, FName(LevelName));


	// 멀티
	//GetWorld()->ServerTravel(LevelName + TEXT("?listen"));;
}
