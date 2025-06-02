#include "Item/ItemData/NAItemData.h"
#include "Item/ItemActor/NAItemActor.h"
#include "Item/EngineSubsystem/NAItemEngineSubsystem.h"

#include "Inventory/NAInventoryComponent.h"

// 프로그램 시작 시 0 에서 시작
FThreadSafeCounter UNAItemData::IDCount(0);

UNAItemData::UNAItemData()
{
	if (!HasAnyFlags(RF_ClassDefaultObject)) {
		IDNumber = IDCount.Increment();
	}
	ID = NAME_None;
}

void UNAItemData::PostInitProperties()
{
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::PostInitProperties] CDO) %s"), *GetName());
		
		if (ID.IsNone())
		{
			UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::PostInitProperties] CDO) %s's ID is none."),*GetName());
		}
	}
	else if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::PostInitProperties]  일반) %s"), *GetName());
		if (ID.IsNone())
		{
			UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::PostInitProperties]  일반) %s's ID is none."),*GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::PostInitProperties]  일반) %s's ID: %s."), *GetName(), *ID.ToString());
			// 예상) 레벨에 배치된 아이템 액터가 에디터 켜질때 처음 로드되는 경우: UNAItemEngineSubsystem의 CreateItemDataByActor를 호출하지 않음
		}
		if (GetOuter() != UNAItemEngineSubsystem::Get())
		{
			UE_LOG(LogTemp, Warning,
				   TEXT(
					   "[UNAItemData::PostInitProperties]  일반) %s's Outer[%s] is NOT UNAItemEngineSubsystem. 억덕계 이런 일이"
				   ), *GetName(), *GetOuter()->GetName());
		}
	}
}

void UNAItemData::SetQuantity(const int32 NewQuantity)
{
	if (NewQuantity != Quantity)
	{
		if (const FNAItemBaseTableRow* ItemMetaData = GetItemMetaDataStruct<FNAItemBaseTableRow>()) {
			Quantity = FMath::Clamp(NewQuantity, 0, ItemMetaData->NumericData.bIsStackable ? ItemMetaData->NumericData.MaxSlotStackSize : 1);
			if (OwningInventory.IsValid())
			{
				if (Quantity <= 0)
				{
					OwningInventory->HandleRemoveSingleItemData(this);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[UNAItemData::SetQuantity]  OwningInventory was null!"));
			}
		}
	}
}

void UNAItemData::SetItemState(EItemState NewItemState)
{
	if (ItemState != NewItemState)
	{
		ItemState = NewItemState;
		// 델리게이트?
	}
}

EItemType UNAItemData::GetItemType() const
{
	if (const FNAItemBaseTableRow* ItemDataStruct = GetItemMetaDataStruct())
	{
		return ItemDataStruct->ItemType;
	}
	return EItemType::IT_None;
}

UClass* UNAItemData::GetItemActorClass() const
{
	if (const FNAItemBaseTableRow* ItemMetaData = GetItemMetaDataStruct())
	{
		return ItemMetaData->ItemClass.Get();
	}
	return nullptr;
}

bool UNAItemData::IsPickableItem() const
{
	if (const FNAItemBaseTableRow* ItemMetaData = GetItemMetaDataStruct())
	{
		return ItemMetaData->ItemType != EItemType::IT_None && ItemMetaData->ItemType != EItemType::IT_Misc;
	}
	return false;
}

bool UNAItemData::IsStackableItem() const
{
	if (const FNAItemBaseTableRow* ItemMetaData = GetItemMetaDataStruct())
	{
		return ItemMetaData->NumericData.bIsStackable;
	}
	return false;
}

int32 UNAItemData::GetItemMaxSlotStackSize() const
{
	if (const FNAItemBaseTableRow* ItemMetaData = GetItemMetaDataStruct())
	{
		return ItemMetaData->NumericData.MaxSlotStackSize;
	}
	return -1;
}

int32 UNAItemData::GetMaxInventoryHoldCount() const
{
	if (const FNAItemBaseTableRow* ItemMetaData = GetItemMetaDataStruct())
	{
		return ItemMetaData->NumericData.MaxInventoryHoldCount;
	}
	return -1;
}
