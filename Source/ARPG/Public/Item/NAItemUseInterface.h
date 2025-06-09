// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NAItemUseInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNAItemUseInterface : public UInterface
{
	GENERATED_BODY()
};

class UNAItemData;

// CDO를 경유하여 호출될 것을 전제함. 전역 함수처럼 쓰일 가상 메서드이기 때문에 반드시 const 함수로 선언되어야 함.
class ARPG_API INAItemUseInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool CanUseItem(UNAItemData* InItemData, AActor* User) const { return false; }
	virtual bool UseItem(UNAItemData* InItemData, AActor* User) const { return false; }
};
