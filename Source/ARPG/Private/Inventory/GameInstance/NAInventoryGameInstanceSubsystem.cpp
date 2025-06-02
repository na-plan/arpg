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
			for (const FNAInventorySlot& InvSlot : Inventory)
			{
				const UNAItemData* NewItemData = UNAItemEngineSubsystem::Get()->CreateItemDataBySlot(GetWorld(), InvSlot);
				if (!NewItemData)
				{
					UE_LOG(LogTemp, Warning, TEXT("[UNAInventoryGameInstanceSubsystem::Initialize]  인벤토리 슬롯으로 아이템 데이터 생성 실패"));
					continue;
				}
				InventoryItems.Emplace(NewItemData->GetItemID(), InvSlot.ItemSlotStack);
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

void UNAInventoryGameInstanceSubsystem::AddItemToInventory(class UNAInventoryComponent* Inventory, const FName& SlotID, const UNAItemData* ItemData)
{
	check(Inventory);
	FNAInventorySlot NewSlotData;
	const bool bResult = MakeSlotData(Inventory, SlotID, ItemData, NewSlotData);
	check(bResult);
	// @TODO: Inventory의 Owner의 컨트롤러의 서버 권한을 나타내는 문자열 형식을 만들고 이를 SlotID와 짬뽕하기
	InventoryItems.Emplace(SlotID, NewSlotData);
}

int32 UNAInventoryGameInstanceSubsystem::RemoveItemFromInventory(const FName& ItemID, int32 Stack)
{
	int32 RemovedItems = 0;
	if (InventoryItems.Contains(ItemID))
	{
		if (InventoryItems[ItemID] <= 0)
		{
			// 뭐임 왜 인벤토리에 있는 아이템인데 스택이 0임??
		}
		
		if (InventoryItems[ItemID] - Stack == 0)
		{
			// 다 삭제 TMap::Remove
		}
		else if (InventoryItems[ItemID] - Stack < 0)
		{
			// 오류? 삭제가 요청된 수량이 인벤토리가 보유한 수량보다 컸음
		}
		else if (InventoryItems[ItemID] - Stack > 0)
		{
			// 인벤토리 상 슬롯은 냅두고, 수량만 줄이기
		}
	}
	return false;
}

bool UNAInventoryGameInstanceSubsystem::MakeSlotData(UNAInventoryComponent* Inventory, const FName& SlotID, const UNAItemData* ItemData, FNAInventorySlot& OutSlotData)
{
	check(IsValid(Inventory) && IsValid(ItemData) && !SlotID.IsNone());
	check(Inventory == ItemData->GetOwningInventory());
	
	OutSlotData.ItemMetaDataKey = ItemData->GetItemActorClass();
	OutSlotData.SlotID = SlotID;
	OutSlotData.ItemState = static_cast<uint8>(ItemData->ItemState);
	OutSlotData.ItemSlotStack = ItemData->Quantity;
	
	return true;
}

void UNAInventoryGameInstanceSubsystem::SortInventory()
{
}

bool UNAInventoryGameInstanceSubsystem::EquipItem(const FName& ItemMetaID)
{
	return false;
}

bool UNAInventoryGameInstanceSubsystem::UnequipItem(const FName& ItemMetaID)
{
	return false;
}

