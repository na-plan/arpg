// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NAGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API ANAGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

	// PlayerState를 추가하기 전에 플레이어의 캐릭터를 부여
	virtual void AddPlayerState(APlayerState* PlayerState) override;
};
