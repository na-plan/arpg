// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/EngineSubsystem/NAItemEngineSubsystem.h"

#include "Inventory/DataStructs/NAInventoryDataStructs.h"

#include "Item/ItemActor/NAItemActor.h"
#include "Item/ItemDataStructs/NAWeaponDataStructs.h"
//#include "Misc/ItemPatchHelper.h"

// 와 이것도 정적 로드로 CDO 생김 ㅁㅊ
UNAItemEngineSubsystem::UNAItemEngineSubsystem()
{
}

void UNAItemEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG( LogInit, Log, TEXT("%hs"), __FUNCTION__ )

	if (ItemDataTableSources.IsEmpty())
	{
		// 1) Registry 에셋 동기 로드
		static const FString RegistryPath = TEXT("/Script/ARPG.ItemDataTablesAsset'/Game/00_ProjectNA/01_Blueprint/00_Actor/MainGame/Items/DA_ItemDataTables.DA_ItemDataTables'");
		UItemDataTablesAsset* Registry = Cast<UItemDataTablesAsset>(StaticLoadObject(UItemDataTablesAsset::StaticClass(), nullptr, *RegistryPath));
	
		if (!Registry)
		{
			UE_LOG(LogTemp, Error, TEXT("[UNAItemGameInstanceSubsystem::Initialize]  ItemDataTablesAsset 로드 실패: %s"), *RegistryPath);
			return;
		}
	
		// 2) Registry 안의 SoftObjectPtr<UDataTable> 리스트 순회
		UE_LOG(LogTemp, Log, TEXT("[UNAItemGameInstanceSubsystem::Initialize]  아이템 DT LoadSynchronous 시작"));
		for (const TSoftObjectPtr<UDataTable>& SoftDT : Registry->ItemDataTables)
		{
			UDataTable* ResourceDT = SoftDT.LoadSynchronous(); // 이때 DT 안에 있던 BP 클래스의 CDO가 생성됨(직렬화까지 완료)
			if (!ResourceDT)
			{
				UE_LOG(LogTemp, Warning,
					TEXT("[UNAItemGameInstanceSubsystem]  Failed to load DataTable: %s"), *SoftDT.ToString());
				continue;
			}
	
			ItemDataTableSources.Emplace(ResourceDT);
			UE_LOG(LogTemp, Log,
				TEXT("[UNAItemGameInstanceSubsystem]  Loaded DataTable: %s"), *ResourceDT->GetName());
		}
	
		// (2) 메타데이터 맵 빌드
		if (ItemMetaDataMap.IsEmpty())
		{
			// ItemMetaDataMap 버킷 확보
			int32 ExpectedCount = 0;
			for (UDataTable* DT : ItemDataTableSources)
			{
				ExpectedCount += DT->GetRowMap().Num();
			}
			ItemMetaDataMap.Reserve(ExpectedCount);
	
			for (UDataTable* DT : ItemDataTableSources)
			{
				for (const TPair<FName, uint8*>& Pair : DT->GetRowMap())
				{
					FName  RowName = Pair.Key;
					FNAItemBaseTableRow* Row = DT->FindRow<FNAItemBaseTableRow>(RowName, TEXT("Mapping item meta data"));
					if (!Row || !Row->ItemClass) { continue; }
					FDataTableRowHandle Handle;
					Handle.DataTable = DT;
					Handle.RowName = RowName;
					ItemMetaDataMap.Emplace(Row->ItemClass.Get(), Handle);
				}
			}
		}
	}
	
	if (!ItemDataTableSources.IsEmpty() && !ItemMetaDataMap.IsEmpty()) {
		bMetaDataInitialized = true;
		UE_LOG(LogTemp, Log, TEXT("[UNAItemEngineSubsystem::Initialize]  아이템 메타데이터 맵 초기화 완료"));
	}
}

void UNAItemEngineSubsystem::Deinitialize()
{
	Super::Deinitialize();
}
#if WITH_EDITOR
bool UNAItemEngineSubsystem::IsRegisteredItemMetaClass(UClass* ItemClass) const
{
	return ItemClass->IsChildOf<ANAItemActor>() && ItemMetaDataMap.Contains(ItemClass);
}

void UNAItemEngineSubsystem::RegisterNewItemMetaData(UClass* NewItemClass, const UDataTable* InDataTable, const FName InRowName)
{
	if (InDataTable && InRowName.IsValid())
	{
		// 재검증
		if (IsRegisteredItemMetaClass(NewItemClass))
		{
			ensure(false);
			return;
		}
		FDataTableRowHandle NewHandle;
		NewHandle.DataTable = InDataTable;
		NewHandle.RowName = InRowName;
		ItemMetaDataMap.Emplace(NewItemClass, NewHandle);
	}
}

void UNAItemEngineSubsystem::VerifyItemMetaDataRowHandle(UClass* ItemClass, const UDataTable* InDataTable, const FName InRowName)
{
	if (IsItemMetaDataInitialized() && IsRegisteredItemMetaClass(ItemClass))
	{
		if (ItemMetaDataMap[ItemClass].IsNull())
		{
			UE_LOG(LogTemp, Warning, TEXT("[VerifyItemMetaDataRowHandle]  왜 어째서 메타데이터 핸들이 null입니까 휴먼."));
			ItemMetaDataMap[ItemClass].DataTable = InDataTable;
			ItemMetaDataMap[ItemClass].RowName = InRowName;
			return;
		}

		if (ItemMetaDataMap[ItemClass].DataTable != InDataTable)
		{
			ensureAlwaysMsgf(false,
				TEXT("[VerifyItemMetaDataRowHandle]  아이템 메타데이터 오류: 아이템 클래스는 동일한데 데이터 테이블이 달랐음"));
			//ItemMetaDataMap[ItemClass].DataTable = InDataTable;
			return;
		}
		
		if (ItemMetaDataMap[ItemClass].RowName != InRowName)
		{
			UE_LOG(LogTemp, Warning, TEXT("[VerifyItemMetaDataRowHandle]  RowName 업데이트: [%s, %s]"),
				*ItemClass->GetName(), *InRowName.ToString());
			ItemMetaDataMap[ItemClass].RowName = InRowName;
		}
	}
}
#endif

UNAItemData* UNAItemEngineSubsystem::CreateItemDataCopy(const UNAItemData* SourceItemData)
{
	if (!IsValid(SourceItemData))
	{
		ensureAlwaysMsgf(false, TEXT("[UNAItemEngineSubsystem::CreateItemDataCopy]  SourceItemData was null."));
		return nullptr;
	}

	if (SourceItemData->ItemMetaDataHandle.IsNull())
	{
		ensureAlwaysMsgf(false, TEXT("[UNAItemEngineSubsystem::CreateItemDataCopy]  SourceItemData's ItemMetaDataHandle was null."));
		return nullptr;
	}
	
	// 1) DuplicateObject로 원본을 복제합니다 (생성자 로직은 실행되지 않음).
	UNAItemData* Duplicated = DuplicateObject<UNAItemData>(SourceItemData, this);
	if (!Duplicated)
	{
		ensureAlwaysMsgf(false, TEXT("[UNAItemEngineSubsystem::CreateItemDataCopy]  Failed to duplicate item data."));
		return nullptr;
	}

	// 2) 생성자가 실행되지 않았으니, IDCount를 수동으로 증가
	//    FThreadSafeCounter::Increment()은 증가된 신규 값을 반환
	int32 NewNumber = UNAItemData::IDCount.Increment();
	Duplicated->IDNumber = NewNumber;

	// 3) ID를 “RowName + NewNumber” 형태로 다시 세팅
	FString NameStr;
	NameStr = Duplicated->ItemMetaDataHandle.RowName.ToString();
	FString CountStr = FString::FromInt(Duplicated->IDNumber);
	FString NewItemID = NameStr + TEXT("_") + CountStr;

	Duplicated->ID = FName(*NewItemID);

	// 4) 새로 생성한 UNAItemData 객체의 소유권을 런타임 때 아이템 데이터 추적용 Map으로 이관
	RuntimeItemDataMap.Emplace(Duplicated->ID, Duplicated);

	UE_LOG(LogTemp, Warning, TEXT("[CreateItemDataCopy]  아이템 데이터 복제(%s), 원본 소스 데이터(%s)")
			, *NewItemID, *SourceItemData->ID.ToString());
	
	return RuntimeItemDataMap[Duplicated->ID].Get();
}

UNAItemData* UNAItemEngineSubsystem::GetRuntimeItemData(const FName& InItemID) const
{
	UNAItemData* Value = nullptr;
	if (!InItemID.IsNone())
	{
		Value = RuntimeItemDataMap.Find(InItemID)->Get();
	}
	return Value;
}

UNAItemData* UNAItemEngineSubsystem::CreateItemDataBySlot(UWorld* InWorld, const FNAInventorySlot& InInventorySlot)
{
	if (InInventorySlot.ItemMetaDataKey)
	{
		FDataTableRowHandle ItemMetaDTRowHandle = *(ItemMetaDataMap.Find(InInventorySlot.ItemMetaDataKey.Get()));
		if (ItemMetaDTRowHandle.IsNull()) {
			ensureAlwaysMsgf(false,
				TEXT("[UNAItemEngineSubsystem::CreateItemDataBySlot]  ItemMetaDataMap에 등록되지 않은 ItemActorClass임.  %s"),
				*InInventorySlot.ItemMetaDataKey.Get()->GetName());
			return nullptr;
		}
		
		UNAItemData* NewItemData = NewObject<UNAItemData>(this, NAME_None, RF_Transient);
		if (!NewItemData) {
			ensureAlwaysMsgf(false, TEXT("[UNAItemEngineSubsystem::CreateItemDataBySlot]  새로운 UNAItemData 객체 생성 실패"));
			return nullptr;
		}
		
		NewItemData->ItemMetaDataHandle = ItemMetaDTRowHandle;
		FString NameStr    = ItemMetaDTRowHandle.RowName.ToString();
		FString CountStr   = FString::FromInt(NewItemData->IDCount.GetValue());
		FString NewItemID  = NameStr + TEXT("_") + CountStr;
		
		NewItemData->ID = FName(*NewItemID);
		NewItemData->ItemState = static_cast<EItemState>(InInventorySlot.ItemState);

		// 3) 새로 생성한 UNAItemData 객체의 소유권을 런타임 때 아이템 데이터 추적용 Map으로 이관
		RuntimeItemDataMap.Emplace(NewItemData->ID, NewItemData);

		UE_LOG(LogTemp, Warning, TEXT("[CreateItemDataBySlot]  슬롯 데이터로 아이템 데이터 생성(%s)")
			, *NewItemID);
		
		return RuntimeItemDataMap[NewItemData->ID];
	}

	ensureAlwaysMsgf(false, TEXT("[UNAItemEngineSubsystem::CreateItemDataBySlot]  InInventorySlot의 ItemMetaDataKey가 유효하지 않음."));
	return nullptr;
}

bool UNAItemEngineSubsystem::DestroyRuntimeItemData(const FName& InItemID, const bool bDestroyItemActor)
{
	bool bResult = RuntimeItemDataMap.Contains(InItemID);
	if (bResult)
	{
		UNAItemData* ItemData = RuntimeItemDataMap[InItemID];
		if (ensureAlways(IsValid(ItemData)))
		{
			ItemData->RemoveFromRoot();
			ItemData->ConditionalBeginDestroy();

			RuntimeItemDataMap[InItemID] =  nullptr;
			int32 bSucceed = RuntimeItemDataMap.Remove(InItemID);
			bResult = bSucceed == 1;
		}
		if (GetWorld() && bDestroyItemActor)
		{
			ForEachItemActorOfClass<ANAItemActor>(GetWorld(), [InItemID](ANAItemActor* ItemActor)
			{
				if (ItemActor->GetItemData()->GetItemID() == InItemID)
				{
					ItemActor->Destroy();
				}
			});
		}
	}
	return bResult;
}

bool UNAItemEngineSubsystem::DestroyRuntimeItemData(UNAItemData* InItemData, const bool bDestroyItemActor)
{
	return DestroyRuntimeItemData(InItemData->ID, bDestroyItemActor);
}
