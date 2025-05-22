// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NAHandActor.generated.h"

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

	inline static const FName LeftHandSocketName = "LeftHandSocket";
	inline static const FName RightHandSocketName = "RightHandSocket";
	inline static const FName HeadSocketName = "HeadSocket";

	virtual UChildActorComponent* GetLeftHandChildActorComponent() const = 0;
	virtual UChildActorComponent* GetRightHandChildActorComponent() const = 0;
};
