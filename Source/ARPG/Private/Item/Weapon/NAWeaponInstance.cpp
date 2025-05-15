#include "Item/Weapon/NAWeaponInstance.h"

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
