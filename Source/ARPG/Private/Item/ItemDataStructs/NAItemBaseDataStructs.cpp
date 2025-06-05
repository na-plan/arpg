#include "Item/ItemDataStructs/NAItemBaseDataStructs.h"
#include "Item/ItemActor/NAWeapon.h"
#include "Item/EngineSubsystem/NAItemEngineSubsystem.h"

FNAItemBaseTableRow::FNAItemBaseTableRow(UClass* InItemClass)
{
	//UE_LOG(LogTemp, Warning, TEXT("[FNAItemBaseTableRow::FNAItemBaseTableRow]  아이템 DT 기본 생성자"));

	if (InItemClass && InItemClass->IsChildOf<ANAItemActor>())
	{
		ItemClass = InItemClass;
	}
}

#if WITH_EDITOR

void FNAItemBaseTableRow::OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName)
{
	FNAItemBaseTableRow* ItemRowStruct = InDataTable->FindRow<FNAItemBaseTableRow>(InRowName, TEXT("On Data Table Changed"));
	
	if (ItemRowStruct == this)
	{
		if (UNAItemEngineSubsystem::Get()
			&& UNAItemEngineSubsystem::Get()->IsItemMetaDataInitialized())
		{
			UClass* ItemActorClass = ItemRowStruct->ItemClass.Get();
			if (ensure(ItemActorClass))
			{
				if (!UNAItemEngineSubsystem::Get()->IsRegisteredItemMetaClass(ItemActorClass))
				{
					UNAItemEngineSubsystem::Get()->RegisterNewItemMetaData(ItemActorClass, InDataTable, InRowName);
				}
				else
				{
					UNAItemEngineSubsystem::Get()->VerifyItemMetaDataRowHandle(ItemActorClass, InDataTable, InRowName);
				}
			}
		}

		if (ItemType == EItemType::IT_Weapon
			|| ItemClass.Get()->IsChildOf<ANAWeapon>())
		{
			if (NumericData.bIsStackable)
			{
				NumericData.MaxSlotStackSize = 1;
				NumericData.MaxInventoryHoldCount = 1;
			}
			else
			{
				NumericData.MaxSlotStackSize = -1;
				NumericData.MaxInventoryHoldCount = -1;
			}
		}
			
		if (!NumericData.bIsStackable)
		{
			NumericData.MaxSlotStackSize = 1;
		}
		
		if (NumericData.MaxInventoryHoldCount < 0)
		{
			NumericData.MaxInventoryHoldCount = 0;
			NumericData.MaxSlotStackSize = 1;
		}
		else if (NumericData.MaxInventoryHoldCount == 0)
		{
			if (!ensure(NumericData.MaxSlotStackSize >= 1))
			{
				NumericData.MaxSlotStackSize = 1;
			}
		}
		else if (NumericData.MaxInventoryHoldCount > 0)
		{
			if (!ensure(NumericData.MaxSlotStackSize >= 1
				&& NumericData.MaxSlotStackSize <= NumericData.MaxInventoryHoldCount))
			{
				NumericData.MaxSlotStackSize = NumericData.MaxInventoryHoldCount;
			}
		}

		if (!InRowName.IsNone()
			&& InRowName.ToString() != TextData.Name.ToString())
		{
			TextData.Name = FText::FromName(InRowName);
		}
	}
}
#endif