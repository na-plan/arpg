// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/GameInstance/NAInventoryGameInstanceSubsystem.h"

#include "Inventory/NAInventoryComponent.h"
#include "Item/EngineSubsystem/NAItemEngineSubsystem.h"

void UNAInventoryGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// 아이템 서브 시스템의 Initialize가 먼저 호출되도록 보장
	check(UNAItemEngineSubsystem::Get());
	
	Super::Initialize(Collection);

	if (GEngine)
	{
		bool bInventoryInitialized = false;
		// @TODO: 세이브 파일에서 인벤토리 데이터를 읽어와서 Inventory를 초기화하는 로직

		// Inventory를 순회하며 InventoryItems를 초기화
		if (bInventoryInitialized)
		{
			InventoryItems.Reset();
			for (const FNAInventorySlot& InvSlot : Inventory)
			{
				UNAItemData* NewItemData = UNAItemEngineSubsystem::Get()->CreateItemDataBySlot(GetWorld(), InvSlot);
				if (!NewItemData)
				{
					UE_LOG(LogTemp, Warning, TEXT("[UNAInventoryGameInstanceSubsystem::Initialize]  인벤토리 슬롯으로 아이템 데이터 생성 실패"));
					continue;
				}
				InventoryItems.Add(NewItemData, InvSlot);
			}
		}
	}
	else
	{
		ensureAlwaysMsgf(false, TEXT("[UNAInventoryGameInstanceSubsystem::Initialize]  아이템 서브시스템의 초기화에 문제가 생김... 아마도"));
	}
}

void UNAInventoryGameInstanceSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UNAInventoryGameInstanceSubsystem::AddItemToInventory(UNAInventoryComponent* InInventory, const FName& SlotID, UNAItemData* ItemData)
{
	check(InInventory);
	FNAInventorySlot NewSlotData;
	const bool bResult = MakeSlotData(InInventory, SlotID, ItemData, NewSlotData);
	check(bResult);
	// @TODO: Inventory의 Owner의 컨트롤러의 서버 권한을 나타내는 무언가를 만들기?
	InventoryItems.Add(ItemData, NewSlotData);
}

UNAItemData* UNAInventoryGameInstanceSubsystem::RemoveItemFromInventory(UNAItemData* ItemData)
{
	const bool bContains = InventoryItems.Contains(ItemData);
	check(bContains);
	InventoryItems.Remove(ItemData);
	return ItemData;
}

bool UNAInventoryGameInstanceSubsystem::MakeSlotData(UNAInventoryComponent* InInventory, const FName& SlotID, const UNAItemData* ItemData, FNAInventorySlot& OutSlotData)
{
	check(IsValid(InInventory) && IsValid(ItemData) && !SlotID.IsNone());
	check(InInventory == ItemData->GetOwningInventory());
	
	OutSlotData.ItemMetaDataKey = ItemData->GetItemActorClass();
	OutSlotData.SlotID = SlotID;
	OutSlotData.ItemState = static_cast<uint8>(ItemData->GetItemState());
	OutSlotData.ItemSlotStack = ItemData->GetQuantity();
	
	return true;
}

