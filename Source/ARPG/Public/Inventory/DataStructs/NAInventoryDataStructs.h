// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NAInventoryDataStructs.generated.h"

/**
 * 인벤토리 슬롯(칸 1개)에 담길 정보: 아이템 아이디(정확히는 아이템 데이터), 해당 슬롯이 들고 있는 아이템 stack
 */
USTRUCT(/*BlueprintType*/)
struct FNAInventorySlot
{
	GENERATED_BODY()

	// 메타데이터 추적에 필요
	UPROPERTY()
	TSubclassOf<class ANAItemActor> ItemMetaDataKey = nullptr;

	UPROPERTY()
	FName							SlotID			= NAME_None;

	UPROPERTY()
	uint8							ItemState		= 0;
	
	UPROPERTY()
	int32							ItemSlotStack	= -1;
};
