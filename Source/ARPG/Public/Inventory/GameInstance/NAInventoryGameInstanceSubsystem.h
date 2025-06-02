// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Inventory/DataStructs/NAInventoryDataStructs.h"
#include "NAInventoryGameInstanceSubsystem.generated.h"

class UNAItemData;
struct FNAInventorySlot;
/**
 * 인벤토리 조작 API를 제공
 * @see: UNAInventoryComponent: 인벤토리 조작을 위한 링커 게층
 */
UCLASS()
class ARPG_API UNAInventoryGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UNAInventoryGameInstanceSubsystem* Get(const UWorld* InWorld)
	{
		if (IsValid(InWorld))
		{
			if (UGameInstance* GI = InWorld->GetGameInstance())
			{
				return GI->GetSubsystem<UNAInventoryGameInstanceSubsystem>();
			}
		}
		return nullptr;
	}
	
	// 인벤토리 조작 API
	void AddItemToInventory(class UNAInventoryComponent* Inventory,const FName& SlotID, const UNAItemData* ItemData);
	int32 RemoveItemFromInventory(const FName& ItemMetaID, int32 Stack = 1);
	
	bool MakeSlotData(UNAInventoryComponent* Inventory, const FName& SlotID, const UNAItemData* ItemData, FNAInventorySlot& OutSlotData);

	void UpdateSlotData(const FName& SlotID, UNAItemData* ItemData);
	
	// 장착/해제
	bool EquipItem(const FName& ItemMetaID);
	bool UnequipItem(const FName& ItemMetaID);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
private:
	// 런타임 중 인벤토리에 소지된 아이템들을 저장한 맵 <아이템 아이디, 아이템 stack>
	// 게임 시작 시 세이브 파일에서 읽어온 인벤토리 아이템 정보를 가지고 Inventory를 초기화
	// Inventory의 값을 바탕으로 UNAItemData를 생성(이건 아이템 서브 시스템에서 생성 및 소유권을 가짐, 아이템 아이디 부여)
	// InventoryItemMap에 방금 생성한 UNAItemData과 생성 기반 정보(from Inventory)를 맵핑하여 저장
	//UPROPERTY()
	//TMap<FName, int32> InventoryItems;

	// 슬롯id, 슬롯 데이터 구조체
	// UPROPERTY()
	// TMap<FName, FNAInventorySlot> InventoryItems;

	// 런타임 중 인벤토리에 소지된 아이템
	// 아이템 데이터, 슬롯ID
	UPROPERTY()
	TMap<TWeakObjectPtr<UNAItemData>, FName> InventoryItems;

	// 세이브 파일 생성 때 보낼 인벤토리 데이터를 작성하기 위한 맵
	// 게임 세이브 파일 생성 시, key 값을 바탕으로(아이템 서브 시스템에서 읽어온 UNAItemData*) FInventorySlot의 값을 변경
	// 그 다음 이 맵의 요소들을 배열로 변환하여 세이브 파일 매니저에게 전달
	UPROPERTY()
	TMap<FName, FNAInventorySlot> InventoryItemMap;
	
	// 세이브 파일의 인벤토리 데이터를 읽기 위해서만 사용할 배열
	// 아이템 메타데이터가 런타임 메모리에 등록된 이후에 이 배열을 초기화할 것
	// 게임 시작 때 InventoryItems와 InventoryItemMap을 초기화하는데 사용된 이후로 쓸일 없음
	UPROPERTY()
	TArray<FNAInventorySlot> Inventory;
};
