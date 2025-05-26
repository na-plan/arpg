// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Item/ItemData/NAItemData.h"
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

class ANAItemActor;
struct FNAInventorySlot;
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
	const ItemDTRow_T* GetItemMetaDataByClass(TSubclassOf<ANAItemActor> InItemActorClass) const;

	template</*typename ItemDTRow_T = FNAItemBaseTableRow, */typename ItemActor_T = ANAItemActor>
		requires /*TIsDerivedFrom<ItemDTRow_T, FNAItemBaseTableRow>::IsDerived && */TIsDerivedFrom<ItemActor_T, ANAItemActor>::IsDerived
	UNAItemData* CreateItemDataByActor(ItemActor_T* InItemActor);
	
	UNAItemData* CreateItemDataBySlot(const FNAInventorySlot& InInventorySlot);

	UNAItemData* GetRuntimeItemData(const FName& Key) const;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void UpdateRuntimeItemDataMap(FName InItemDataID, UNAItemData* InItemData);
	void ValidateItemRow(const FNAItemBaseTableRow* RowPtr, const FName RowName);

private:
	// 실제 사용할 DataTable 포인터 보관
	UPROPERTY()
	TArray<UDataTable*> ItemDataTableSources;

	// 메타데이터 매핑
	UPROPERTY()
	TMap<TSubclassOf<ANAItemActor>, FDataTableRowHandle> ItemMetaDataMap;
	//TMap<TSubclassOf<ANAItemActor>, TPair<UScriptStruct*, FDataTableRowHandle> ItemMetaDataMap;
	UPROPERTY()
	uint8 bIsMetaMapInitialized : 1 = false;


	// 런타임 데이터 매핑
	// 아이템 ID: 런타임 때 아이템 데이터 식별용
	UPROPERTY()
	TMap<FName, TObjectPtr<UNAItemData>> RuntimeItemDataMap;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemDataCreated, FName, UNAItemData*)
extern FOnItemDataCreated OnItemDataCreated;

template<typename ItemDTR_T>
	requires TIsDerivedFrom<ItemDTR_T, FNAItemBaseTableRow>::IsDerived
inline const ItemDTR_T* UNAItemGameInstanceSubsystem::GetItemMetaDataByClass(TSubclassOf<ANAItemActor> InItemActorClass) const
{
	const FDataTableRowHandle* Value = ItemMetaDataMap.Find(InItemActorClass);
	if (Value)
	{
		return Value->GetRow<ItemDTR_T>(Value->RowName.ToString());
	}
	
	return nullptr;
}

template</*typename ItemDTRow_T, */typename ItemActor_T>
	requires /*TIsDerivedFrom<ItemDTRow_T, FNAItemBaseTableRow>::IsDerived &&*/ TIsDerivedFrom<ItemActor_T, ANAItemActor>::IsDerived
inline UNAItemData* UNAItemGameInstanceSubsystem::CreateItemDataByActor(ItemActor_T* InItemActor)
{
	if (!IsItemMetaDataReady()) {
		ensureAlwaysMsgf(false, TEXT("[UNAItemGameInstanceSubsystem::CreateItemDataByActor]  아직도 아이템 메타데이터 매핑이 안되어있다고?? 어째서야"));
		return nullptr;
	}

	if (InItemActor ) {
		UClass* InItemActorClass = InItemActor->GetClass();

		if (InItemActorClass->GetDefaultObject() == InItemActor) {
			ensureAlwaysMsgf(false, TEXT("[UNAItemGameInstanceSubsystem::CreateItemDataByActor]  InItemActor에 CDO 객체를 전달하면 안됨~~!. %s"), *InItemActorClass->GetName());
			return nullptr;
		}

		// 1) 아이템 메타데이터 검색
		TMap<TSubclassOf<ANAItemActor>, FDataTableRowHandle>::ValueType* ValuePtr = ItemMetaDataMap.Find(InItemActorClass);
		if (!ValuePtr)
		{
			UE_LOG(LogTemp, Warning, TEXT("[UNAItemGameInstanceSubsystem::CreateItemDataByActor]  ValuePtr was null."));
			return nullptr;
		}
		FDataTableRowHandle ItemMetaDTRowHandle = *ValuePtr;
		if (ItemMetaDTRowHandle.IsNull()) {
			ensureAlwaysMsgf(false, TEXT("[UNAItemGameInstanceSubsystem::CreateItemDataByActor]  ItemMetaDataMap에 등록되지 않은 ItemActorClass임.  %s"), *InItemActorClass->GetName());
			return nullptr;
		}

		// 2) 매핑된 DataTableRowHandle을 이용하여 Data Table Row 값 조회하기
		//ItemDTRow_T* ItemMetaData = ItemMetaDTRowHandle.GetRow<ItemDTRow_T>(ItemMetaDTRowHandle.RowName.ToString());
		//if (!ItemMetaData) {
		//	ensureAlwaysMsgf(false, TEXT("[UNAItemGameInstanceSubsystem::CreateItemDataByActor]  ItemMetaDataMap[%s]의 값이 유효하지 않음."), *InItemActorClass->GetName());
		//	return nullptr;
		//}

		// 3) UNAItemData 객체 생성 및 초기화
		UNAItemData* NewItemData = NewObject<UNAItemData>(this, NAME_None, RF_Transient);
		if (!NewItemData) {
			ensureAlwaysMsgf(false, TEXT("[UNAItemGameInstanceSubsystem::CreateItemDataByActor]  새로운 UNAItemData 객체 생성 실패"));
			return nullptr;
		}
		NewItemData->ItemMetaDataHandle = ItemMetaDTRowHandle;
		FString NameStr    = ItemMetaDTRowHandle.RowName.ToString();
		FString CountStr   = FString::FromInt(NewItemData->IDCount.GetValue());
		FString NewItemID  = NameStr + TEXT("_") + CountStr;
		
		NewItemData->ID = FName(*NewItemID);

		// 4) 아이템 데이터 추적용 Map에 새로 생성한 UNAItemData 객체의 소유권 이전
		RuntimeItemDataMap.Emplace(NewItemData->ID, NewItemData);

		return NewItemData;
	}

	ensureAlwaysMsgf(false, TEXT("[UNAItemGameInstanceSubsystem::CreateItemDataByActor]  유효하지 않은 ItemActor를 전달받음."));
	return nullptr;
}