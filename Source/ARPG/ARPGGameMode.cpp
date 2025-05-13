// Copyright Epic Games, Inc. All Rights Reserved.

#include "ARPGGameMode.h"

#include "NAInGameHUD.h"
#include "NAPlayerController.h"
#include "Public/NAPlayerState.h"
#include "UObject/ConstructorHelpers.h"

AARPGGameMode::AARPGGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	
	PlayerStateClass = ANAPlayerState::StaticClass();
	PlayerControllerClass = ANAPlayerController::StaticClass();
	HUDClass = ANAInGameHUD::StaticClass();
}
