// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/NALobbyGameMode.h"

#include "Lobby/NALobbyGameState.h"
#include "Lobby/NALobbyHUD.h"

ANALobbyGameMode::ANALobbyGameMode()
{
	HUDClass = ANALobbyHUD::StaticClass();
	DefaultPawnClass = nullptr;
}
