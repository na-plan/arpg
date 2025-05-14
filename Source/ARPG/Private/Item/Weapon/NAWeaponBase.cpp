#include "Item/Weapon/NAWeaponBase.h"

ANAWeaponBase::ANAWeaponBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool ANAWeaponBase::TryGetWeaponData(FNAWeaponTableRow& OutDataTableRow) const
{
	if (ItemDataTableRowHandle.IsNull()) { return false; }
	const FNAWeaponTableRow* DT = GetItemData<FNAWeaponTableRow>(ItemDataTableRowHandle);
	if (!DT) { return false; }
	OutDataTableRow = *DT;
	return true;
}

void ANAWeaponBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

bool ANAWeaponBase::SetData(const FDataTableRowHandle& InDataTableRowHandle)
{
	const bool bSucceed = Super::SetData(InDataTableRowHandle);
	if (!bSucceed) { return false; }

	FNAWeaponTableRow* WeaponData = GetItemData<FNAWeaponTableRow>(ItemDataTableRowHandle);
	if (!WeaponData) { return false; }

}

bool ANAWeaponBase::IsCompatibleDataTable(const FDataTableRowHandle& InDataTableRowHandle) const
{
	if (InDataTableRowHandle.IsNull()) { return false; }

	const UDataTable* DT = InDataTableRowHandle.DataTable;
	if (!DT) { return false; }

	const UScriptStruct* RowStruct = DT->GetRowStruct();
	if (!RowStruct) { return false; }

	bool bIsCompatible = RowStruct == FNAWeaponTableRow::StaticStruct();
	return bIsCompatible;
}
