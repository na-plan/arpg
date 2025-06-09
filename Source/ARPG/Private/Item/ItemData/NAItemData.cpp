#include "Item/ItemData/NAItemData.h"

#include "AbilitySystemInterface.h"
#include "Item/ItemActor/NAItemActor.h"
#include "Item/EngineSubsystem/NAItemEngineSubsystem.h"

#include "Inventory/NAInventoryComponent.h"
#include "Item/NAItemUseInterface.h"

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
			Quantity = FMath::Clamp(NewQuantity, 0,
				ItemMetaData->NumericData.bIsStackable ? ItemMetaData->NumericData.MaxSlotStackSize : 1);
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

FString UNAItemData::GetItemName() const
{
	if (const FNAItemBaseTableRow* ItemMetaData = GetItemMetaDataStruct())
	{
		return ItemMetaData->TextData.Name.ToString();
	}
	return {};
}

class UTexture2D* UNAItemData::GetItemIcon() const
{
	if (const FNAItemBaseTableRow* ItemMetaData = GetItemMetaDataStruct())
	{
		return ItemMetaData->IconAssetData.ItemIcon;
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

void UNAItemData::SetOwningInventory(UNAInventoryComponent* NewInventory)
{
	if (NewInventory != nullptr)
	{
		OwningInventory = NewInventory;
	}
}

bool UNAItemData::TryUseItem(AActor* User)
{
	UClass* ItemClass = GetItemActorClass();
	if (!ItemClass) return false;

	UObject* CDO = ItemClass->GetDefaultObject(false);
	if (!CDO) return false;

	bool bSucceed = false;
	if (INAItemUseInterface* ItemUseInterface = Cast<INAItemUseInterface>(CDO))
	{
		bSucceed = ItemUseInterface->UseItem(this, User);
		if (bSucceed)
		{
			if (Quantity <= 0)
			{
				// @TODO: 이 아이템 데이터 파괴 및 후속처리(RuntimeItemDataMap에서 제거 등)
			}
		
			if (OwningInventory.IsValid())
			{
				// @TODO: 인벤토리 위젯에 리드로우 해야하는 상황(아이템 수량 및 상태 변경 등)이면 인벤토리 컴포넌트에 위젯 리드로우 요청
				
			}
		}
	}
	return bSucceed;
}
