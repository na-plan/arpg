// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/GameInstance/NAItemGameInstanceSubsystem.h"

#include "Item/ItemData/NAItemData.h"
#include "Item/ItemInstance/NAItemInstance.h"

FOnItemDataCreated OnItemDataCreated;

// UItemDataTablesAsset 파일 하나 만든 다음에 주석 풀기
void UNAItemGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	//Super::Initialize(Collection);
	//OnItemDataCreated.AddUObject(this, &UNAItemGameInstanceSubsystem::UpdateItemDataMap);

	//// 1) Registry 에셋 동기 로드 (나중에 실제 경로로 교체)
	//static const FString RegistryPath = TEXT("아이템 메타데이터 DB의 소스로 쓰일 DT들을 모아둔 데이터 에셋: 에디터에서 편집");
	//UItemDataTablesAsset* Registry = Cast<UItemDataTablesAsset>(StaticLoadObject(UItemDataTablesAsset::StaticClass(), nullptr, *RegistryPath));

	//if (!Registry)
	//{
	//	UE_LOG(LogTemp, Error, TEXT("[UNAItemGameInstanceSubsystem::Initialize]  ItemDataTablesAsset 로드 실패: %s"), *RegistryPath);
	//	return;
	//}

	//// 2) Registry 안의 SoftObjectPtr<UDataTable> 리스트 순회
	//for (const TSoftObjectPtr<UDataTable>& SoftDT : Registry->ItemDataTables)
	//{
	//	UDataTable* DT = SoftDT.LoadSynchronous();
	//	if (!DT)
	//	{
	//		UE_LOG(LogTemp, Warning,
	//			TEXT("[UNAItemGameInstanceSubsystem]  Failed to load DataTable: %s"), *SoftDT.ToString());
	//		continue;
	//	}

	//	ItemDataTableSources.Emplace(DT);
	//	UE_LOG(LogTemp, Log,
	//		TEXT("[UNAItemGameInstanceSubsystem]  Loaded DataTable: %s"), *DT->GetName());
	//}

	//// (2) 메타데이터 맵 빌드
	//ItemMetaDataMap.Empty();
	//for (UDataTable* DT : ItemDataTableSources)
	//{
	//	for (const TPair<FName, uint8*>& Pair : DT->GetRowMap())
	//	{
	//		FName  RowName = Pair.Key;
	//		FNAItemBaseTableRow* Row = DT->FindRow<FNAItemBaseTableRow>(RowName, TEXT("Mapping item meta data"));
	//		if (Row && Row->ItemClass)
	//		{
	//			FDataTableRowHandle Handle;
	//			Handle.DataTable = DT;
	//			Handle.RowName = RowName;
	//			ItemMetaDataMap.Emplace(Row->ItemClass, Handle);
	//		}
	//	}
	//}

	//if (!ItemMetaDataMap.IsEmpty()) {
	//	bIsMetaMapInitialized = true;
	//}

	/*for (const auto [Name, Ptr] : Items)
	{
		ANAItemInstance* ItemInstance = NewObject<ANAItemInstance>(GetWorld(), ANAItemInstance::StaticClass(), NAME_None, RF_Transactional);
	}*/

	// 블루프린트 (아이템) - 매시 지정, 아이템 효과 지정, 기타 등등 -> (Item 클래스)
	// 테이블에 모든 아이템을 넣어 놓고 -> ( 아이템을 저장할 테이블 Row, TSubClassof<Item클래스> + 아이템 설명 등등 (Ability 제외, GAS로 조회가능) )
	// 게임 인스턴스에서 테이블을 순회하면서 메타 데이터를 생성 -> ( 아이템 인스턴스에 Item 클래스, 아이템 설명 복사)

	// 레벨에 있는 아이템 = 그냥 있으면 됌 (얘내는 굳이 메타 데이터가 필요 없음 ㅇㅇ...)
	// 레벨에 있는 아이템의 설명 = 메타 데이터를 게임 인스턴스에서 조회하고 보여주면 됨 (아이템 -> 메타 데이터)
	// 인벤토리 = 메타 데이터로 변환 후 배열에 저장 (아이템 -> 메타 데이터)
	// 인벤토리 -> 드랍 = 메타 데이터로 아이템 클래스를 조회 후 SpawnActor (메타 데이터 -> 아이템)

}

void UNAItemGameInstanceSubsystem::Deinitialize()
{
	OnItemDataCreated.RemoveAll(this);
	Super::Deinitialize();
}

void UNAItemGameInstanceSubsystem::UpdateItemDataMap(FName InItemDataID, UNAItemData* InItemData)
{
	if (!InItemDataID.IsNone()) {
		ensureAlwaysMsgf(false, TEXT("[UNAItemGameInstanceSubsystem::UpdateItemDataMap]  InItemDataID가 유효하지 않음."));
	}

	if (!InItemData) {
		ensureAlwaysMsgf(false, TEXT("[UNAItemGameInstanceSubsystem::UpdateItemDataMap]  InItemData가 유효하지 않음."));
		return;
	}

	if (RuntimeItemDataMap.Contains(InItemDataID)) {
		ensureAlwaysMsgf(false, TEXT("[UNAItemGameInstanceSubsystem::UpdateItemDataMap]  InItemDataID가 이미 존재함."));
		return;
	}

}
