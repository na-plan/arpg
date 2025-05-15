#pragma once

#include "Item/NAItemBaseDataStructs.h"
#include "NAItemData.generated.h"

//=============================================================================
// IMPLEMENT_TRYGETROW 매크로 사용시 주의사항:
//  - StructType    : 반드시 FNAItemBaseTableRow 파생 타입이어야 함!
//  - FuncName      : "TryGet~~MetaData" 형식의 함수명으로 작성해야 합니다.
//  - CategoryName  : 반드시 문자열 리터럴("...") 형태로 전달해야 합니다.
//=============================================================================
#define IMPLEMENT_TRYGETROW(StructType, FuncName, CategoryName)\
	UFUNCTION(BlueprintCallable, Category = CategoryName)\
	bool FuncName(StructType& OutRow) const\
	{\
		if (const StructType* DT = GetItemMetaData<StructType>(ItemDataTableRowHandle))\
		{\
			OutRow = *DT;\
			return true;\
		}\
		return false;\
	}\
//=============================================================================

UENUM(BlueprintType)
enum class EItemState : uint8
{
	IS_None            UMETA(DisplayName = "None"),           // 초기화 필요

	IS_Acquired        UMETA(DisplayName = "Acquired"),       // 인벤토리에 획득된 상태
	IS_Active          UMETA(DisplayName = "Active"),         // 사용 가능 상태
	IS_Inactive        UMETA(DisplayName = "Inactive"),       // 사용 불가 (조건 불충족, 쿨다운 등)
	IS_Consumed        UMETA(DisplayName = "Consumed"),       // 사용 후 사라진 상태
	IS_Disabled        UMETA(DisplayName = "Disabled"),       // 의도적으로 비활성화된 상태
	IS_Locked          UMETA(DisplayName = "Locked"),         // 특정 조건을 만족해야 사용할 수 있음
	IS_Expired         UMETA(DisplayName = "Expired"),        // 유효기간 경과 등으로 무효 상태
	IS_Hidden          UMETA(DisplayName = "Hidden"),          // UI 등에서 숨김 처리
	IS_Equipped          UMETA(DisplayName = "Equipped"),          // 착용 중인 장비 아이템 전용
	IS_Broken          UMETA(DisplayName = "Broken"),          // 내구도가 0이 되어 작동하지 않는 상태
	IS_Pending          UMETA(DisplayName = "Pending"),          // 네트워크 처리 중이거나, UI 연출 중 잠시 대기 상태
	IS_PreviewOnly          UMETA(DisplayName = "PreviewOnly"),          // 월드에 있지만 상호작용할 수 없는 상태 (예: 상점 프리뷰)
};

class UInventoryComponent;
UCLASS()
class ARPG_API UNAItemData final : public UObject
{
	GENERATED_BODY()

public:
	UNAItemData();
	//UNAItemData(const FObjectInitializer& ObjectInitializer);

	// Editor에서 Default 세팅해 주지 않았을 때 사용할 “기본” 핸들
	// 파생 클래스에서 기본 핸들의 값이 별도로 설정되었다면, 이 함수도 오버라이딩 필요
	//virtual FDataTableRowHandle GetDefaultRowHandle();

	virtual void PostInitProperties() override;

	static UNAItemData* CreateItemData(UObject* Outer = nullptr, FName RowName, UScriptStruct* InItemMetaData);

	// 런타임에서 UNAItemData를 생성할 때 사용하는 팩토리 함수
	// 반드시!! DataTableRowHandle을 설정해야 함
	//template<typename ItemDataT = UNAItemData> requires TIsDerivedFrom<ItemDataT, UNAItemData>::IsDerived
	//static ItemDataT* CreateItemData(UObject* Outer = nullptr, TSubclassOf<UNAItemData> NewItemDataClass, const FDataTableRowHandle& InDataTableRowHandle);

	// 런타임에서 UNAItemData를 복사생성할 때 사용하는 팩토리 함수
	// 반드시!! 복사 대상 객체(InOriginal)를 전달해야 함
	//template<typename ItemDataT = UNAItemData>
	//static ItemDataT* CloneItemData(UObject* Outer = nullptr, ItemDataT* InOriginal);

	//UFUNCTION(BlueprintCallable, Category = "Item Data")
	//UNAItemData* CloneItemData(UObject* Outer = nullptr, UNAItemData* InOriginal, UClass* InOriginalClass,const FDataTableRowHandle& InDataTableRowHandle);


	UFUNCTION(Category = "Item")
	FORCEINLINE float GetItemStackWeight() const  // 아이템 스택 무게
	{
		if (const FNAItemBaseTableRow* ItemData = GetItemMetaData<FNAItemBaseTableRow>(ItemDataTableRowHandle)) {
			return Quantity * ItemData->NumericData.ItemWeight;
		}
		else {
			return -1.0f;
		}
	}

	UFUNCTION(Category = "Item")
	FORCEINLINE float GetItemWeight() const	// 아이템 무게
	{
		if (const FNAItemBaseTableRow* ItemData = GetItemMetaData<FNAItemBaseTableRow>(ItemDataTableRowHandle)) {
			return  ItemData->NumericData.ItemWeight;
		}
		else {
			return -1.0f;
		}
	}

	UFUNCTION(Category = "Item")
	FORCEINLINE bool IsFullItemStack() const  // 아이템 스택이 가득 찼는지 여부
	{
		if (const FNAItemBaseTableRow* ItemData = GetItemMetaData<FNAItemBaseTableRow>(ItemDataTableRowHandle)) {
			return  Quantity == ItemData->NumericData.MaxStackSize;
		}
		else {
			return false;
		}
	}

	UFUNCTION(Category = "Item")
	void SetQuantity(const int32 NewQuantity);

	UFUNCTION(Category = "Item")
	virtual void UseItem(AActor* User);	// 아이템 사용

	template<typename ItemMetaDataT = FNAItemBaseTableRow>
	const ItemMetaDataT* GetItemMetaData(const FDataTableRowHandle& InDataTableRowHandle) const;

	IMPLEMENT_TRYGETROW(FNAItemBaseTableRow, TryGetItemMetaData, "Item Data")

protected:
	bool operator==(const FName& OtherID) const
	{
		return ID == OtherID;
	}

	//virtual bool LoadItemData(const FNAItemBaseTableRow& InDataTableRow);
	virtual bool IsCompatibleDataTable(const FDataTableRowHandle& InDataTableRowHandle) const;

private:
	/** 객체가 생성될 때마다 ++ 하여 ID 를 뽑아 주는 원자적 카운터 */
	static FThreadSafeCounter IDCount;
	int32 IDNumber = -1;

public:
	UPROPERTY(VisibleAnywhere, Category = "Item Data", meta = (UIMin = 1, UIMax = 100))
	int32 Quantity = 1;	// 아이템의 최대 수량

	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	FName ID;  // 아이템 ID (DT 행 이름 + 숫자)

	UPROPERTY(EditAnywhere, Category = "Item Data")
	EItemState ItemState = EItemState::IS_None;

	//UPROPERTY(VisibleAnywhere, Category = "Item Ownership")
	//TWeakObjectPtr<UInventoryComponent> OwningInventory = nullptr;	// 현재 소유 인벤토리

	UPROPERTY(EditAnywhere, Category = "Item Data")
	TWeakObjectPtr< UScriptStruct> ItemMetaData = nullptr;

	UPROPERTY(EditAnywhere, Category = "Item Data")
	FDataTableRowHandle ItemDataTableRowHandle;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemDataCreated, FName, UNAItemData*)
extern FOnItemDataCreated OnItemDataCreated;

UNAItemData* UNAItemData::CreateItemData(UObject* Outer, FName RowName, UScriptStruct* InItemMetaData)
{
	if (RowName.IsNone()) {
		ensureAlwaysMsgf(false, TEXT("[UNAItemData::CreateItemData]  RowName이 유효하지 않음."));
		return nullptr;
	}

	if (!InItemMetaData) {
		ensureAlwaysMsgf(false, TEXT("[UNAItemData::CreateItemData]  InItemMetaData가 유효하지 않음."));
		return nullptr;
	}

	UNAItemData* NewItemData = nullptr;
	UObject* NewOuter = Outer;
	if (!NewOuter) {
		// Fix: Use GEngine->GetWorldContexts() to get a valid UWorld reference
		UWorld* OuterWorld = nullptr;
		if (GEngine)
		{
			const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
			if (WorldContexts.Num() > 0)
			{
				OuterWorld = WorldContexts[0].World();
			}
		}
		if (OuterWorld) {
			NewOuter = OuterWorld;
		}
		else {
			ensureAlwaysMsgf(false, TEXT("[UNAItemData::CreateItemData]  OuterWorld이 유효하지 않음."));
			return nullptr;
		}
	}

	NewItemData = NewObject<UNAItemData>(NewOuter, NAME_None, RF_Transactional);
	if (!NewItemData) {
		ensureAlwaysMsgf(false, TEXT("[UNAItemData::CreateItemData]  NewItemData 생성 실패."));
		return nullptr;
	}

	NewItemData->ItemMetaData = InItemMetaData;
	FString ItemID = RowName.ToString() + FString::Printf(TEXT("_%d"), NewItemData->IDCount);
	NewItemData->ID = FName(*ItemID);
	OnItemDataCreated.Broadcast(NewItemData->ID, NewItemData);
	return NewItemData;
}

//template<typename ItemDataT> requires TIsDerivedFrom<ItemDataT, UNAItemData>::IsDerived
//static ItemDataT* UNAItemData::CreateItemData(UObject* Outer = nullptr, TSubclassOf<UNAItemData> NewItemDataClass, const FDataTableRowHandle& InDataTableRowHandle)
//{
//	/*static_assert(TIsDerivedFrom<ItemDataT, UNAItemData>::IsDerived,
//		"[UNAItemData::CreateItemData]  ItemDataT는 반드시 UNAItemData의 파생 클래스이어야 합니다.");*/
//
//	UClass* ItemDataClass = ItemDataT::StaticClass();
//	if (ItemDataClass != NewItemDataClass.Get()) {
//		UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::CreateItemData]  ItemDataClass와 NewItemDataClass가 다름. %s, %s"), *ItemDataClass->GetName(), *NewItemDataClass->GetName());
//		return nullptr;
//	}
//
//	if (!InDataTableRowHandle.IsNull()) {
//		bool bIsCompatibleDataTable = ItemDataClass->GetDefaultObject()->IsCompatibleDataTable(InDataTableRowHandle);
//		if (!bIsCompatibleDataTable) {
//			//UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::CreateItemData]  아이템 클래스 전용 DT를 쓰셈.  %s, %s"), *ItemDataT::StaticClass()->GetName(), *InDataTableRowHandle.ToDebugString());
//			return nullptr;
//		}
//	}
//
//	ItemDataT* NewItemData = nullptr;
//	if (Outer) {
//		NewItemData = NewObject<ItemDataT>(Outer, NAME_None, RF_Transactional);
//	}
//	else {
//		ULevel* OuterLevel = GetWorld()->GetCurrentLevel();
//		UObject* NewOuter = OuterLevel ? OuterLevel : (UObject*)GetTransientPackage();
//		NewItemData = NewObject<ItemDataT>(NewOuter, NAME_None, RF_Transactional);
//	}
//
//}

//template<typename ItemDataT>
//ItemDataT* UNAItemData::CloneItemData(UObject* Outer, ItemDataT* InOriginal)
//{
//	static_assert(TIsDerivedFrom<ItemDataT, UNAItemData>::IsDerived,
//		"[UNAItemData::CloneItemData]  ItemDataT는 반드시 UNAItemData의 파생 클래스이어야 합니다.");
//
//	if (!InOriginal) {
//		UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::CloneItemData]  InOriginal이 유요하지 않음."));
//		return nullptr;
//	}
//
//	if (InOriginal->ItemDataTableRowHandle.IsNull()) {
//		UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::CloneItemData]  InOriginal의 ItemDataTableRowHandle이 유효하지 않음."));
//		return nullptr;
//	}
//
//	ItemDataT* NewItemData = nullptr;
//	if (Outer) {
//		NewItemData = NewObject<ItemDataT>(Outer, NAME_None, RF_Transactional);
//	}
//	else {
//		ULevel* OuterLevel = GetWorld()->GetCurrentLevel();
//		UObject* NewOuter = OuterLevel ? OuterLevel : (UObject*)GetTransientPackage();
//		NewItemData = NewObject<ItemDataT>(NewOuter, NAME_None, RF_Transactional);
//	}
//
//	if (!NewItemData ) {
//		UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::CloneItemData]  NewItemData 복사생성 실패. %s"), *InOriginal->GetClass()->GetName());
//		return nullptr;
//	}
//
//	NewItemData->Quantity = InOriginal->Quantity;
//	NewItemData->ID = InOriginal->ID;
//	NewItemData->ItemState = InOriginal->ItemState;
//	NewItemData->ItemDataTableRowHandle = InOriginal->ItemDataTableRowHandle;
//
//	return NewItemData;
//}

template<typename ItemMetaDataT>
const ItemMetaDataT* UNAItemData::GetItemMetaData(const FDataTableRowHandle& InDataTableRowHandle) const
{
	static_assert(TIsDerivedFrom<ItemMetaDataT, FNAItemBaseTableRow>::IsDerived,
		"[UNAItemData::GetItemMetaData]  GetItemMetaData can only be used to get FNAItemBaseTableRow instances.");

	if (!InDataTableRowHandle.DataTable)
	{
		return nullptr;
	}

	ItemMetaDataT* ItemMetaData = InDataTableRowHandle.GetRow<ItemMetaDataT>(InDataTableRowHandle.RowName.ToString());
	if (!ItemMetaData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::GetItemMetaData]  아이템 데이터 테이블 읽기 실패.  %s"), *InDataTableRowHandle.ToDebugString());
		return nullptr;
	}

	return ItemMetaData;
}