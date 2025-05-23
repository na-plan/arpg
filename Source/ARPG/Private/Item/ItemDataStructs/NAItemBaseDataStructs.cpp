#include "Item/ItemDataStructs/NAItemBaseDataStructs.h"

void FNAItemBaseTableRow::OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName)
{
	//FTableRowBase::OnDataTableChanged(InDataTable, InRowName);
	
	FNAItemBaseTableRow* ItemRowStruct = InDataTable->FindRow<FNAItemBaseTableRow>(InRowName, TEXT("On Data Table Changed"));
	if (ItemRowStruct)
	{
		if (ItemRowStruct->NumericData.MaxInventoryStackSize < 0)
		{
			ItemRowStruct->NumericData.MaxInventoryStackSize = FMath::Max(ItemRowStruct->NumericData.MaxInventoryStackSize, -1);
			ItemRowStruct->NumericData.MaxSlotStackSize = FMath::Max(ItemRowStruct->NumericData.MaxInventoryStackSize, -1);
		}
		else if (ItemRowStruct->NumericData.MaxInventoryStackSize == 0)
		{
			ItemRowStruct->NumericData.MaxSlotStackSize = 0;
		}
		else if (ItemRowStruct->NumericData.MaxInventoryStackSize > 0)
		{
			if (ItemRowStruct->NumericData.MaxSlotStackSize <= 0)
			{
				ItemRowStruct->NumericData.MaxSlotStackSize = ItemRowStruct->NumericData.MaxInventoryStackSize;
			}
			else if (ItemRowStruct->NumericData.MaxInventoryStackSize < ItemRowStruct->NumericData.MaxSlotStackSize)
			{
				ItemRowStruct->NumericData.MaxSlotStackSize = ItemRowStruct->NumericData.MaxInventoryStackSize;
			}
		}
	}
}
