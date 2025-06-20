// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NAGameMode.generated.h"

UCLASS(minimalapi)
class ANAGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANAGameMode();
	
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

public:
	UPROPERTY()
	int32 PlayerCount;
};



