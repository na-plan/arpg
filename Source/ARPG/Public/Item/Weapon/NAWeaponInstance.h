#pragma once

#include "Item/Weapon/NAWeaponData.h"
#include "Item/NAItemInstance.h"
#include "NAWeaponInstance.generated.h"

UCLASS()
class ARPG_API ANAWeaponBase : public ANAItemBase
{
	GENERATED_BODY()

public:
	ANAWeaponBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Weapon Data")
	bool TryGetWeaponData(FNAWeaponTableRow& OutDataTableRow) const;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual bool SetData(const FDataTableRowHandle& InDataTableRowHandle) override;

	virtual bool IsCompatibleDataTable(const FDataTableRowHandle& InDataTableRowHandle) const override;

public:
};