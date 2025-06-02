// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "NAInventoryComponent.generated.h"


UENUM(BlueprintType)
enum class ENAItemAddStatus : uint8
{
	IAR_NoItemAdded				UMETA(DisplayName = "No Item Added"),
	IAR_PartialAmountItemAdded	UMETA(DisplayName = "Partial Amount Item Added"),
	IAR_AllItemAdded			UMETA(DisplayName = "All Item Added"),
};

USTRUCT(BlueprintType)
struct FNAItemAddResult
{
	GENERATED_BODY()

	FNAItemAddResult():
	ActualAmountAdded(0),
	OperationResult(ENAItemAddStatus::IAR_NoItemAdded),
	ResultMessage(FText::GetEmpty()) {}

	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	int32			ActualAmountAdded;

	// 아이템 Add의 결과를 캐시하는 이넘 변수
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	ENAItemAddStatus	OperationResult;

	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	FText			ResultMessage;

	static FNAItemAddResult AddedNone(const FText& ErrorText)
	{
		FNAItemAddResult AddedNoneResult;
		AddedNoneResult.ActualAmountAdded = 0;
		AddedNoneResult.OperationResult = ENAItemAddStatus::IAR_NoItemAdded;
		AddedNoneResult.ResultMessage = ErrorText;

		return AddedNoneResult;
	}
	static FNAItemAddResult AddedPartial(const int32 PartialAmountAdded, const FText& ErrorText)
	{
		FNAItemAddResult AddedPartialResult;
		AddedPartialResult.ActualAmountAdded = PartialAmountAdded;
		AddedPartialResult.OperationResult = ENAItemAddStatus::IAR_PartialAmountItemAdded;
		AddedPartialResult.ResultMessage = ErrorText;

		return AddedPartialResult;
	}
	static FNAItemAddResult AddedAll(const int32 AmountAdded, const FText& Message)
	{
		FNAItemAddResult AddedAllResult;
		AddedAllResult.ActualAmountAdded = AmountAdded;
		AddedAllResult.OperationResult = ENAItemAddStatus::IAR_AllItemAdded;
		AddedAllResult.ResultMessage = Message;

		return AddedAllResult;
	}
};

//DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemAdded, FItemAddResult);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdated, UNAInventoryComponent*);

class UNAItemData;
/**
 * Handle~: UNAInventoryGameInstanceSubsystem가 제공하는 인벤토리 APT에 대한 래퍼 메서드
 * 위젯 컴포넌트: BeginPlay 때 IniWidget 실행 -> CreateWidget (고정되어있다)
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_API UNAInventoryComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNAInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
// 아이템 수량 데이터 관리 ////////////////////////////////////////////////////////////////////////////////////////////////
	/** 
	 * 새로운 아이템을 빈 슬롯에 등록 or 원래 있던 아이템과 동일한 종류면 기존 슬롯을 먼저 메꾸고
	 * @return add에 성공하고 남은 아이템 수량. 0이면 전부 추가 성공, 0보다 크면 부분 추가 성공, -1이면 전부 추가 실패
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	int32 TryAddItem(UNAItemData* ItemToAdd);

	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	bool TryRemoveItemFromInventory(UNAItemData* ItemToRemove);

	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	bool TryRemoveItemFromSlot(const FName& SlotID, UNAItemData* ItemToRemove);

	bool HasSameItemClass(UClass* ClassToCheck, TArray<FName>* OutSlotList = nullptr) const;

	/*
	 * @return	-1: 슬롯 검색 실패 or 빈 슬롯이었음
	 *			0: 남은 스택 가능량 없음
	 */
	int32 GetRemainingStackCapacityInSlot(const FName& SlotID) const;

	bool IsEmptySlot(const FName& SlotID) const;
	
	/** 빈 Inventory(Inven_) 슬롯들을 반환 */
	void GetEmptyInventorySlotIDs(TArray<FName>& OutEmptySlots) const;
	/** 빈 Weapon(Weapon_) 슬롯들을 반환*/
	void GetEmptyWeaponSlotIDs(TArray<FName>& OutEmptySlots) const;

	int32 GetCurrentTotalHoldCount(UClass* ClassToCheck) const;

	bool IsSlotFull(const FName& SlotID) const;
	// 빈 슬롯이 없는지 여부. 인벤토리의 전체 용량(각 슬롯의 스택 크기를 포함)은 검사하지 않음
	bool IsOutOfSlot() const;
	/** 전체 인벤토리 용량이 다 찼는지 여부 */
	bool IsInventoryFull() const;


protected:
	// 인벤토리 슬롯에 스택 불가능한 아이템
	// @return 추가에 성공한 수량
	FNAItemAddResult AddNonStackableItem_Internal(UNAItemData* InputItem, const TArray<FName>& InEmptySlots);

	// 슬롯에 넘치고 남은 아이템 수량이 있다면 가장 가까운 슬롯에 남은 아이템을 보관
	// 만약 남은 아이템을 넣을 슬롯이 없다면 이후 분기처리
	// @return 추가에 성공한 수량
	FNAItemAddResult AddStackableItem_Internal(UNAItemData* InputItem, const TArray<FName>& InEmptySlots);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	bool HandleAddNewItem(UNAItemData* NewItemToAdd, const FName& SlotID);

	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	void HandleRemoveSingleItemData(UNAItemData* ItemDataToRemove);

	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	int32 HandleRemoveAmountOfItems(UNAItemData* ItemToRemove, int32 DesiredAmountToRemove);

	// 인벤토리 정렬 함수
	void SortInventoryItems();

public:
// 아이템 수량 관리를 위한 유틸 함수 ///////////////////////////////////////////////////////////////////////////////////////
	
	// @TODO: 인벤토리 무게 계산 방법 정립부터 한 뒤, 로직 구체화
	// UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	// FORCEINLINE float GetInventoryTotalWeight() const
	// {
	// 	return InventoryTotalWeight;
	// }
	
	// @TODO: 인벤토리 무게 계산 방법 정립부터 한 뒤, 로직 구체화
	// UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	// FORCEINLINE float GetInventoryWeightCapacity() const
	// {
	// 	return InventoryWeightCapacity;
	// }
	
	// @TODO: 인벤토리 무게 계산 방법 정립부터 한 뒤, 로직 구체화
	// UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	// FORCEINLINE int32 GetInventorySlotCapacity() const
	// {
	// 	return InventorySlotsCapacity;
	// }
	
	//UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	//int32 CalculateWeightAddAmount(UNAItemData* InItem, int32 RequestedAddAmount);

	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	int32 CalculateNumberForFullSlotStack(const FName& SlotID) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	FName FindMatchingItem(UNAItemData* InItem) const;

	// UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	// UNAItemData* FindNextPartialStack(UNAItemData* InItem) const;

// 아이템 정렬 & 인벤토리 위젯 /////////////////////////////////////////////////////////////////////////////////////////////	
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	void SplitExistingStack(UNAItemData* ItemToSplit, const int32 AmountToSplit);

// Getters & Setters ///////////////////////////////////////////////////////////////////////////////////////////////////
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	TArray<UNAItemData*> GetInventoryContents() const;
	
	// UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	// FORCEINLINE float GetSlotsCapacity() const { return InventorySlotsCapacity; }
	
	//UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	//FORCEINLINE float GetWeightCapacity() const { return InventoryWeightCapacity; }

	//UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	//FORCEINLINE void SetSlotsCapacity(const int32 NewSlotsCapacity)
	//{
	//	InventorySlotsCapacity = NewSlotsCapacity;
	//}
	//UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	//FORCEINLINE void SetWeightCapacity(const float NewWeightCapacity)
	//{
	//	InventoryWeightCapacity =  NewWeightCapacity;
	//}

protected:
	//UPROPERTY(VisibleAnywhere, Category = "Inventory Component")
	//float InventoryTotalWeight;

	//UPROPERTY(VisibleAnywhere, Category = "Inventory Component")
	//int32 InventorySlotsCapacity;

	//UPROPERTY(VisibleAnywhere, Category = "Inventory Component")
	//float InventoryWeightCapacity;

	// SlotID, ItemData
	UPROPERTY(VisibleAnywhere, Category = "Inventory Component")
	TMap<FName, TWeakObjectPtr<UNAItemData>> InventoryContents;

	// 이 인벤토리 위젯이 보유한 슬롯의 총 개수
	static constexpr int32 MaxInventorySlots = 25;
	static constexpr int32 MaxWeaponSlots = 4;
	static constexpr int32 MaxTotalSlots = MaxInventorySlots + MaxWeaponSlots;

	// @TODO: 장착 중인 수트의 레벨에 따라 인벤토리의 용량이 다르게 설계하기

public:
/////////////////////////////////////////////////////////////////////////////////////////////////////
//  Inventory Widget  ///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
	void ReleaseInventoryWidget();
	void CollapseInventoryWidget();
	
	
protected:
	UPROPERTY(Transient)
	uint8 bInventoryWidgetVisible : 1 = false;
};
