// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NAPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API ANAPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	// 현재 체력
	float GetHealth() const;

	// 최대 체력
	int32 GetMaxHealth() const;

	// 생존 상태 반환
	bool IsAlive() const;

protected:

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
