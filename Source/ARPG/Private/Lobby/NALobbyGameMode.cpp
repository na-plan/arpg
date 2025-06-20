// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/NALobbyGameMode.h"
#include "NAGameStateBase.h"
#include "NAPlayerController.h"
#include "NAPlayerState.h"
#include "Lobby/NALobbyHUD.h"

ANALobbyGameMode::ANALobbyGameMode()
{
	DefaultPawnClass = nullptr;
	HUDClass = ANALobbyHUD::StaticClass();
	PlayerStateClass = ANAPlayerState::StaticClass();
	GameStateClass = ANAGameStateBase::StaticClass();
	PlayerControllerClass = ANAPlayerController::StaticClass();
}

void ANALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
}
//
// void ANALobbyGameMode::OnClick_SingleMode()
// {
// 	ChangeLevel(GetWorld(),TEXT("/Script/Engine.World'/Game/00_ProjectNA/02_Level/Level_NATestLevel.Level_NATestLevel'"));
// }
//
// void ANALobbyGameMode::OnClick_CoopMode()
// {
// }
// void ANALobbyGameMode::ChangeLevel(UObject* WorldContext, FString LevelName)
// {
// 	UGameplayStatics::OpenLevel(WorldContext, FName(LevelName));
//
// 	
// 	// 멀티
// 	//GetWorld()->ServerTravel(LevelName + TEXT("?listen"));;
// }
