#pragma once

#include "Item/NAItemBaseData.h"
#include "NAItemBase.generated.h"

UCLASS()
class ARPG_API ANAItemBase : public AActor
{
	GENERATED_BODY()

public:
	ANAItemBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Item Data")
	bool TryGetItemData(FNAItemBaseTableRow& OutDataTableRow) const;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual bool SetData(const FDataTableRowHandle& InDataTableRowHandle);

	virtual bool IsCompatibleDataTable(const FDataTableRowHandle& InDataTableRowHandle) const;

public:
	UPROPERTY(EditAnywhere, Category = "Item Data")
	FDataTableRowHandle ItemDataTableRowHandle;
	
protected:
	// 필요시 SetData에서 생성된 후 RootComponent로 설정됨
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Trigger Sphere")
	TObjectPtr<class USphereComponent> TriggerSphere = nullptr;

	UPROPERTY(VisibleAnywhere, Category="Item Mesh")
	TSubclassOf<UMeshComponent> ItemMeshClass;

	// Static Mesh가 기본값
	UPROPERTY(VisibleAnywhere, Category = "Item Mesh")
	TObjectPtr<UStaticMeshComponent> ItemStaticMesh;

	// 필요시 SetData에서 생성됨, 기본값으로 설정된 ItemStaticMesh를 비활성화
	UPROPERTY(VisibleAnywhere, Category = "Item Mesh")
	TObjectPtr<USkeletalMeshComponent> ItemSkeletalMesh = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Item State")
	EItemState ItemState = EItemState::IS_None;

	// 현재 소유자
	UPROPERTY(VisibleAnywhere, Category = "Item Ownership")
	TWeakObjectPtr<AActor> CurrentOwner = nullptr;	
};

template<typename ItemDataT>
ItemDataT* GetItemData(const FDataTableRowHandle& InDataTableRowHandle)
{
	static_assert(TIsDerivedFrom<ItemDataT, FNAItemBaseTableRow>::IsDerived,
		"GetItemData can only be used to get FNAItemBaseTableRow instances.");

	if (!InDataTableRowHandle.DataTable)
	{
		return nullptr;
	}

	ItemDataType* ItemData = InDataTableRowHandle.GetRow<ItemDataType>(InDataTableRowHandle.RowName.ToString());
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GetItemData]  아이템 데이터 테이블 읽기 실패. %s"), *InDataTableRowHandle.RowName.ToString());
		return nullptr;
	}

	return ItemData;
}