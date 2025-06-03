// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "NAInventoryComponent.generated.h"


UENUM(BlueprintType)
enum class ENAItemAddStatus : uint8
{
	IAR_NoItemAdded				UMETA(DisplayName = "No Item Added"),
	IAR_AddedPartial	UMETA(DisplayName = "Partial Amount Item Added"),
	IAR_AddedAll			UMETA(DisplayName = "All Item Added"),
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
		AddedPartialResult.OperationResult = ENAItemAddStatus::IAR_AddedPartial;
		AddedPartialResult.ResultMessage = ErrorText;

		return AddedPartialResult;
	}
	static FNAItemAddResult AddedAll(const int32 AmountAdded, const FText& Message)
	{
		FNAItemAddResult AddedAllResult;
		AddedAllResult.ActualAmountAdded = AmountAdded;
		AddedAllResult.OperationResult = ENAItemAddStatus::IAR_AddedAll;
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
	 * @return add에 성공하고 남은 아이템 수량. 0이면 전부 추가, 0보다 크면 부분 추가, -1이면 전부 추가 실패
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	int32 TryAddItem(UNAItemData* ItemToAdd);

	/** 
	 * @return remove에 성공한 아이템 데이터. 수량에 따라 원본 or 복제본일 수 있음
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	UNAItemData* TryRemoveItem(const FName& SlotID, int32 RequestedAmount);

	/** 인벤토리에 해당 아이템 클래스가 있는지 확인 */
	bool HasItemOfClass(const UClass* ItemClass) const;

	/** 해당 아이템 클래스를 가진 슬롯의 ID 리스트 반환 */
	void GetSlotIDsWithItemClass(const UClass* ItemClass, TArray<FName>& OutSlotIDs) const;
	
	/**
	 * 해당 슬롯의 남은 스택 가능량(=추가로 담을 수 있는 아이템 개수)을 반환
	 * @param SlotID    검사할 슬롯의 ID
	 * @return          -1: 슬롯이 없거나(검색 실패), 슬롯이 비어있음
	 *                   0: 남은 스택 가능량 없음(슬롯이 가득 참)
	 *                  >0: 남은 스택 가능량
	*/
	int32 GetSlotRemainingStack(const FName& SlotID) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	bool IsEmptySlot(const FName& SlotID) const;
	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	bool IsFullSlot(const FName& SlotID) const;
	
	/** 빈 Inventory(Inven_) 슬롯들을 반환 */
	void GatherEmptyInventorySlots(TArray<FName>& OutEmptySlots) const;
	/** 빈 Weapon(Weapon_) 슬롯들을 반환*/
	void GatherEmptyWeaponSlots(TArray<FName>& OutEmptySlots) const;

	/**
	 * 인벤토리에 빈 슬롯이 하나도 없는지 확인
	 * (각 슬롯의 스택 용량과 무관하게, 슬롯 자체가 모두 점유되어 있으면 true)
	 * @return true : 모든 슬롯에 아이템이 들어 있음 (빈 슬롯 없음)
	 *         false: 하나 이상의 빈 슬롯 존재
	*/
	bool HasNoEmptySlot() const;
	/**
	 * 전체 인벤토리가 "더 이상 단 1개도 추가할 수 없는 상태"인지 확인
	 * (모든 슬롯이 점유 중이며, 각 슬롯의 최대 용량까지 모두 차 있음)
	 * @return true  : 인벤토리 완전 가득 참 (추가 불가)
	 *         false : 비어있거나, 최대치가 아닌 슬롯이 하나라도 있으면
	*/
	bool IsAtFullCapacity() const;

protected:
	// Partial 슬롯 목록을 반환 (동일 클래스가 들어 있고, 아직 MaxSlotStackSize만큼 차지 않은 슬롯)
	void GatherPartialSlots(UClass* ItemClass, TArray<FName>& OutPartialSlots) const;

	// Empty 슬롯 목록을 반환 (Weapon vs Inven 구분은 ItemClass 기준으로 판단)
	void GatherEmptySlots(UClass* ItemClass, TArray<FName>& OutEmptySlots ) const;

	// Partial 슬롯과 Empty 슬롯을 모아, 실제로 추가할 수 있는 최대 수량을 계산
	int32 ComputeDistributableAmount(UNAItemData* InputItem, const TArray<FName>& PartialSlots, const TArray<FName>& EmptySlots) const;

	// 실제 Partial → Empty 순으로 슬롯에 수량을 반영
	// 반환된 “실제 추가된 양”을 리턴
	int32 DistributeToSlots(UNAItemData* InputItem, int32 AmountToDistribute,const TArray<FName>& PartialSlots,const TArray<FName>& EmptySlots);

	/**
	 * Non-stackable 아이템이 들어갈 수 있는지 간단히 검증하고, 실패 사유를 문자열로 채워줌
	 * Helper: Non-stackable 아이템 검증
	 *  - nullptr 체크
	 *  - MaxSlotStackSize == 1
	 *  - Quantity == 1
	 *  - MaxInventoryHoldCount 검사 (1 이상일 때, 이미 해당 클래스가 있으면 안 됨)
	*/
	bool IsValidForNonStackable(UNAItemData* InputItem, FString& OutFailReason) const;
	
	// 인벤토리 슬롯에 스택 불가능한 아이템
	// @return 추가에 성공한 수량
	FNAItemAddResult AddNonStackableItem(UNAItemData* InputItem, const TArray<FName>& InEmptySlots);

	// 슬롯에 넘치고 남은 아이템 수량이 있다면 가장 가까운 슬롯에 남은 아이템을 보관
	// 만약 남은 아이템을 넣을 슬롯이 없다면 이후 분기처리
	// @return 추가에 성공한 수량
	FNAItemAddResult AddStackableItem(UNAItemData* InputItem, const TArray<FName>& PartialSlots, const TArray<FName>& EmptySlots);

private:
	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	bool HandleAddNewItem(UNAItemData* NewItemToAdd, const FName& SlotID);

	// 인벤토리에서 완전히 제거
	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	bool HandleRemoveItem(const FName& SlotID);

public:
// 아이템 수량 관리를 위한 유틸 함수 //////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * 해당 슬롯이 최대 용량이 될 때까지 추가로 넣을 수 있는 아이템 개수를 반환
	 * @param SlotID    검사할 슬롯의 ID
	 * @return          -1: 슬롯이 비어있거나 없음
	 *                   0: 슬롯이 이미 가득 참
	 *                  >0: 최대 용량까지 추가할 수 있는 개수
	*/
	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	int32 GetNumToFillSlot(const FName& SlotID) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	UNAItemData* FindMatchingItemByClass(const UClass* ItemClass) const;

// 아이템 정렬 & 인벤토리 위젯 /////////////////////////////////////////////////////////////////////////////////////////////	
	
	// 인벤토리 정렬 함수
	void SortInventoryItems();
	
	//UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	//void SplitExistingStack(UNAItemData* ItemToSplit, const int32 AmountToSplit);

// Getters & Setters ///////////////////////////////////////////////////////////////////////////////////////////////////
	
	UFUNCTION(BlueprintCallable, Category = "Inventory Component")
	TArray<UNAItemData*> GetInventoryContents() const;

protected:
	// SlotID, ItemData
	UPROPERTY(VisibleAnywhere, Category = "Inventory Component")
	TMap<FName, TWeakObjectPtr<UNAItemData>> InventoryContents;

	// 이 인벤토리 위젯이 보유한 슬롯의 총 개수
	static constexpr int32 MaxInventorySlots = 25;
	static constexpr int32 MaxWeaponSlots = 4;
	static constexpr int32 MaxTotalSlots = MaxInventorySlots + MaxWeaponSlots;

	// @TODO: 장착 중인 수트의 레벨에 따라 인벤토리의 총 용량이 달라지도록

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
