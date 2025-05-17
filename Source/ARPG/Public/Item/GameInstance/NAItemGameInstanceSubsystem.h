// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Item/ItemDataStructs/NAItemBaseDataStructs.h"
#include "NAItemGameInstanceSubsystem.generated.h"


UCLASS(BlueprintType)
class UItemDataTablesAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// 에디터에서 드래그&드롭으로 DataTable을 넣을 수 있음
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSoftObjectPtr<UDataTable>> ItemDataTables;
};

class ANAItemInstance;
class UNAItemData;
struct FNAItemBaseTableRow;
/**
 *
 */
UCLASS()
class ARPG_API UNAItemGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	FORCEINLINE bool IsItemMetaDataReady() const {
		return bIsMetaMapInitialized;
	}

	template<typename ItemDTRow_T = FNAItemBaseTableRow>
		requires TIsDerivedFrom<ItemDTRow_T, FNAItemBaseTableRow>::IsDerived
	ItemDTRow_T* GetItemMetaData(TSubclassOf<ANAItemInstance> InItemInstanceClass) const;

	template<typename ItemDTRow_T = FNAItemBaseTableRow, typename ItemInstance_T = ANAItemInstance>
		requires TIsDerivedFrom<ItemDTRow_T, FNAItemBaseTableRow>::IsDerived && TIsDerivedFrom< ANAItemInstance, ItemInstance_T>::IsDerived
	UNAItemData* CreateItemData(ItemInstance_T* InItemInstance);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void UpdateItemDataMap(FName InItemDataID, UNAItemData* InItemData);

private:
	// 실제 사용할 DataTable 포인터 보관
	UPROPERTY()
	TArray<UDataTable*> ItemDataTableSources;

	// 메타데이터 매핑
	UPROPERTY()
	TMap<TSubclassOf<ANAItemInstance>, FDataTableRowHandle> ItemMetaDataMap;
	UPROPERTY()
	uint8 bIsMetaMapInitialized : 1 = false;


	// 런타임 데이터 매핑
	UPROPERTY()
	TMap<FName, TObjectPtr<UNAItemData>> RuntimeItemDataMap;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemDataCreated, FName, UNAItemData*)
extern FOnItemDataCreated OnItemDataCreated;

template<typename ItemDTR_T>
	requires TIsDerivedFrom<ItemDTR_T, FNAItemBaseTableRow>::IsDerived
inline ItemDTR_T* UNAItemGameInstanceSubsystem::GetItemMetaData(TSubclassOf<ANAItemInstance> InItemInstanceClass) const
{
	return nullptr;
}

template<typename ItemDTRow_T, typename ItemInstance_T>
	requires TIsDerivedFrom<ItemDTRow_T, FNAItemBaseTableRow>::IsDerived&& TIsDerivedFrom< ANAItemInstance, ItemInstance_T>::IsDerived
inline UNAItemData* UNAItemGameInstanceSubsystem::CreateItemData(ItemInstance_T* InItemInstance)
{
	if (!IsItemMetaDataReady()) {
		ensureAlwaysMsgf(false, TEXT("[UNAItemGameInstanceSubsystem::CreateItemData]  아직도 아이템 메타데이터 매핑이 안되어있다고?? 어째서야"));
		return nullptr;
	}

	if (InItemInstance ) {
		UClass* InItemInsClass = InItemInstance::StaticClass();

		if (InItemInsClass->GetDefaultObject() == InItemInstance) {
			ensureAlwaysMsgf(false, TEXT("[UNAItemGameInstanceSubsystem::CreateItemData]  InItemInstance에 CDO 객체를 전달하면 안됨~~!. %s"), *InItemInsClass->GetName());
			return nullptr;
		}

		// 1) 아이템 메타데이터 검색
		FDataTableRowHandle ItemMetaDTRowHandle = ItemMetaDataMap.Find(InItemInsClass);
		if (ItemMetaDTRowHandle.IsNull()) {
			ensureAlwaysMsgf(false, TEXT("[UNAItemGameInstanceSubsystem::CreateItemData]  ItemMetaDataMap에 등록되지 않은 ItemInstanceClass임.  %s"), *InItemInsClass->GetName());
			return nullptr;
		}

		// 2) 매핑된 DataTableRowHandle을 이용하여 Data Table Row 값 조회하기
		ItemDTRow_T* ItemMetaData = ItemMetaDTRowHandle.GetRow<ItemMetaDTRowHandle>(ItemMetaDTRowHandle.RowName.ToString());
		if (!ItemMetaData) {
			ensureAlwaysMsgf(false, TEXT("[UNAItemGameInstanceSubsystem::CreateItemData]  ItemMetaDataMap[%s]의 값이 유효하지 않음."), *InItemInsClass->GetName());
			return nullptr;
		}

		// 3) UNAItemData 객체 생성 및 초기화
		UNAItemData* NewItemData = NewObject<UNAItemData>(this, NAME_None, RF_Transient);
		if (!NewItemData) {
			ensureAlwaysMsgf(false, TEXT("[UNAItemGameInstanceSubsystem::CreateItemData]  새로운 UNAItemData 객체 생성 실패"));
			return nullptr;
		}
		NewItemData->ItemMetaDataHandle = ItemMetaDTRowHandle;
		FString NewItemID = ItemMetaDataHandle.RowName.ToString() + FString::Printf(TEXT("_%d"), NewItemData->IDCount);
		NewItemData->ID = FName(*NewItemID);

		// 4) 아이템 데이터 추적용 Map에 새로 생성한 UNAItemData 객체의 소유권 이전
		RuntimeItemDataMap.Emplace(NewItemData->ID, NewItemData);

		return NewItemData;
	}

	ensureAlwaysMsgf(false, TEXT("[UNAItemGameInstanceSubsystem::CreateItemData]  유효하지 않은 ItemInstance를 전달받음."));
	return nullptr;
}
