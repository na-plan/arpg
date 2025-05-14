// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"

#include "NALobbyGameState.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API ANALobbyGameState : public AGameState
{
	GENERATED_BODY()

public:
	bool CheckNetworkConnection() const { return IsConnectedServer; }
	
protected:
	virtual void HandleMatchIsWaitingToStart() override;

	virtual void HandleMatchHasStarted() override;
	
private:
	bool IsConnectedServer = false;
};
