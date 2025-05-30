#include "Item/ItemDataStructs/NAItemBaseDataStructs.h"
#include "Item/ItemActor/NAItemActor.h"

FNAItemBaseTableRow::FNAItemBaseTableRow(UClass* InItemClass)
{
	//UE_LOG(LogTemp, Warning, TEXT("[FNAItemBaseTableRow::FNAItemBaseTableRow]  아이템 DT 기본 생성자"));

	if (InItemClass && InItemClass->IsChildOf<ANAItemActor>())
	{
		ItemClass = InItemClass;
	}
}

void FNAItemBaseTableRow::OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName)
{
	FNAItemBaseTableRow* ItemRowStruct = InDataTable->FindRow<FNAItemBaseTableRow>(InRowName, TEXT("On Data Table Changed"));
	
	if (ItemRowStruct == this)
	{
		if (NumericData.MaxInventoryStackSize < 0)
		{
			NumericData.MaxInventoryStackSize = FMath::Max(NumericData.MaxInventoryStackSize, -1);
			NumericData.MaxSlotStackSize = FMath::Max(NumericData.MaxInventoryStackSize, -1);
		}
		else if (NumericData.MaxInventoryStackSize == 0)
		{
			NumericData.MaxSlotStackSize = 0;
		}
		else if (NumericData.MaxInventoryStackSize > 0)
		{
			if (NumericData.MaxSlotStackSize <= 0)
			{
				NumericData.MaxSlotStackSize = NumericData.MaxInventoryStackSize;
			}
			else if (NumericData.MaxInventoryStackSize < NumericData.MaxSlotStackSize)
			{
				NumericData.MaxSlotStackSize = NumericData.MaxInventoryStackSize;
			}
		}
	}
}
