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
	// �ʿ�� SetData���� ������ �� RootComponent�� ������
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Trigger Sphere")
	TObjectPtr<class USphereComponent> TriggerSphere = nullptr;

	UPROPERTY(VisibleAnywhere, Category="Item Mesh")
	TSubclassOf<UMeshComponent> ItemMeshClass;

	// Static Mesh�� �⺻��
	UPROPERTY(VisibleAnywhere, Category = "Item Mesh")
	TObjectPtr<UStaticMeshComponent> ItemStaticMesh;

	// �ʿ�� SetData���� ������, �⺻������ ������ ItemStaticMesh�� ��Ȱ��ȭ
	UPROPERTY(VisibleAnywhere, Category = "Item Mesh")
	TObjectPtr<USkeletalMeshComponent> ItemSkeletalMesh = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Item State")
	EItemState ItemState = EItemState::IS_None;

	// ���� ������
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
		UE_LOG(LogTemp, Warning, TEXT("[GetItemData]  ������ ������ ���̺� �б� ����. %s"), *InDataTableRowHandle.RowName.ToString());
		return nullptr;
	}

	return ItemData;
}