// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "UObject/Interface.h"
#include "NAHandActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnHandChanged );

// This class does not need to be modified.
UINTERFACE()
class UNAHandActor : public UInterface
{
	GENERATED_BODY()
};

/**
 * 손의 Child Actor를 가지고 있는 객체의 인터페이스
 */
class ARPG_API INAHandActor
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	FOnHandChanged OnHandChanged;

	// 왼손 무기가 장착될 소켓의 이름
	inline static const FName LeftHandSocketName = "LeftHandSocket";
	// 오른손 무기가 장착될 소켓의 이름
	inline static const FName RightHandSocketName = "RightHandSocket";
	// 무기를 발사할때 기준이 될 소켓의 이름 (예: 히트스캔의 시작지점)
	inline static const FName HeadSocketName = "HeadSocket";

	// 왼손 무기의 Child Actor Component
	virtual UChildActorComponent* GetLeftHandChildActorComponent() const = 0;

	// 오른손 무기의 Child Actor Component
	virtual UChildActorComponent* GetRightHandChildActorComponent() const = 0;
};
