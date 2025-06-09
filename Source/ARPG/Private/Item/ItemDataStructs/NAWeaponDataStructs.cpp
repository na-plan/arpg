#include "Item/ItemDataStructs/NAWeaponDataStructs.h"

FNAWeaponTableRow::FNAWeaponTableRow()
{
	ItemType =  EItemType::IT_Weapon;
	NumericData.bIsStackable = false;
	NumericData.MaxSlotStackSize = 1;
	NumericData.MaxInventoryHoldCount = 1;
}
