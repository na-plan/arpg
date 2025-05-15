// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/GameInstance/NAItemGameInstanceSubsystem.h"

#include "Item/NAItemInstance.h"

void UNAItemGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	OnItemDataCreated.AddUObject(this, &UNAItemGameInstanceSubsystem::UpdateItemDataMap);

	if (UDataTable* DataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("'"))))
	{
		ItemDataTables.Emplace(DataTable);
	}
	// ItemDataTables에 있는 모든 DT의 RowMap을 순회하여 ItemMetaDataMap로 이동? 복사?
	// ItemMetaDataMap[RowName(FName), Row Struct(UScriptStruct)] 
	// 
	
	//const TMap<FName, uint8*>& Items = ItemDataTable->GetRowMap();
	
	for (const auto [Name, Ptr] : Items)
	{
		ANAItemInstance* ItemInstance = NewObject<ANAItemInstance>(GetWorld(), ANAItemInstance::StaticClass(), NAME_None, RF_Transactional);
		
	}

	// 블루프린트 (아이템) - 매시 지정, 아이템 효과 지정, 기타 등등 -> (Item 클래스)
	// 테이블에 모든 아이템을 넣어 놓고 -> ( 아이템을 저장할 테이블 Row, TSubClassof<Item클래스> + 아이템 설명 등등 (Ability 제외, GAS로 조회가능) )
	// 게임 인스턴스에서 테이블을 순회하면서 메타 데이터를 생성 -> ( 아이템 인스턴스에 Item 클래스, 아이템 설명 복사)

	// 레벨에 있는 아이템 = 그냥 있으면 됌 (얘내는 굳이 메타 데이터가 필요 없음 ㅇㅇ...)
	// 레벨에 있는 아이템의 설명 = 메타 데이터를 게임 인스턴스에서 조회하고 보여주면 됨 (아이템 -> 메타 데이터)
	// 인벤토리 = 메타 데이터로 변환 후 배열에 저장 (아이템 -> 메타 데이터)
	// 인벤토리 -> 드랍 = 메타 데이터로 아이템 클래스를 조회 후 SpawnActor (메타 데이터 -> 아이템)
	
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

	if (ItemDataMap.Contains(InItemDataID)) {
		ensureAlwaysMsgf(false, TEXT("[UNAItemGameInstanceSubsystem::UpdateItemDataMap]  InItemDataID가 이미 존재함."));
		return;
	}

	ItemDataMap.Emplace(InItemDataID, InItemData);
}
