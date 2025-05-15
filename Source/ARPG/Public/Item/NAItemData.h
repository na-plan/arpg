#pragma once

#include "Item/NAItemBaseDataStructs.h"
#include "NAItemData.generated.h"

//=============================================================================
// IMPLEMENT_TRYGETROW ��ũ�� ���� ���ǻ���:
//  - StructType    : �ݵ�� FNAItemBaseTableRow �Ļ� Ÿ���̾�� ��!
//  - FuncName      : "TryGet~~MetaData" ������ �Լ������� �ۼ��ؾ� �մϴ�.
//  - CategoryName  : �ݵ�� ���ڿ� ���ͷ�("...") ���·� �����ؾ� �մϴ�.
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
	IS_None            UMETA(DisplayName = "None"),           // �ʱ�ȭ �ʿ�

	IS_Acquired        UMETA(DisplayName = "Acquired"),       // �κ��丮�� ȹ��� ����
	IS_Active          UMETA(DisplayName = "Active"),         // ��� ���� ����
	IS_Inactive        UMETA(DisplayName = "Inactive"),       // ��� �Ұ� (���� ������, ��ٿ� ��)
	IS_Consumed        UMETA(DisplayName = "Consumed"),       // ��� �� ����� ����
	IS_Disabled        UMETA(DisplayName = "Disabled"),       // �ǵ������� ��Ȱ��ȭ�� ����
	IS_Locked          UMETA(DisplayName = "Locked"),         // Ư�� ������ �����ؾ� ����� �� ����
	IS_Expired         UMETA(DisplayName = "Expired"),        // ��ȿ�Ⱓ ��� ������ ��ȿ ����
	IS_Hidden          UMETA(DisplayName = "Hidden"),          // UI ��� ���� ó��
	IS_Equipped          UMETA(DisplayName = "Equipped"),          // ���� ���� ��� ������ ����
	IS_Broken          UMETA(DisplayName = "Broken"),          // �������� 0�� �Ǿ� �۵����� �ʴ� ����
	IS_Pending          UMETA(DisplayName = "Pending"),          // ��Ʈ��ũ ó�� ���̰ų�, UI ���� �� ��� ��� ����
	IS_PreviewOnly          UMETA(DisplayName = "PreviewOnly"),          // ���忡 ������ ��ȣ�ۿ��� �� ���� ���� (��: ���� ������)
};

class UInventoryComponent;
UCLASS()
class ARPG_API UNAItemData final : public UObject
{
	GENERATED_BODY()

public:
	UNAItemData();
	//UNAItemData(const FObjectInitializer& ObjectInitializer);

	// Editor���� Default ������ ���� �ʾ��� �� ����� ���⺻�� �ڵ�
	// �Ļ� Ŭ�������� �⺻ �ڵ��� ���� ������ �����Ǿ��ٸ�, �� �Լ��� �������̵� �ʿ�
	//virtual FDataTableRowHandle GetDefaultRowHandle();

	virtual void PostInitProperties() override;

	static UNAItemData* CreateItemData(UObject* Outer = nullptr, FName RowName, UScriptStruct* InItemMetaData);

	// ��Ÿ�ӿ��� UNAItemData�� ������ �� ����ϴ� ���丮 �Լ�
	// �ݵ��!! DataTableRowHandle�� �����ؾ� ��
	//template<typename ItemDataT = UNAItemData> requires TIsDerivedFrom<ItemDataT, UNAItemData>::IsDerived
	//static ItemDataT* CreateItemData(UObject* Outer = nullptr, TSubclassOf<UNAItemData> NewItemDataClass, const FDataTableRowHandle& InDataTableRowHandle);

	// ��Ÿ�ӿ��� UNAItemData�� ��������� �� ����ϴ� ���丮 �Լ�
	// �ݵ��!! ���� ��� ��ü(InOriginal)�� �����ؾ� ��
	//template<typename ItemDataT = UNAItemData>
	//static ItemDataT* CloneItemData(UObject* Outer = nullptr, ItemDataT* InOriginal);

	//UFUNCTION(BlueprintCallable, Category = "Item Data")
	//UNAItemData* CloneItemData(UObject* Outer = nullptr, UNAItemData* InOriginal, UClass* InOriginalClass,const FDataTableRowHandle& InDataTableRowHandle);


	UFUNCTION(Category = "Item")
	FORCEINLINE float GetItemStackWeight() const  // ������ ���� ����
	{
		if (const FNAItemBaseTableRow* ItemData = GetItemMetaData<FNAItemBaseTableRow>(ItemDataTableRowHandle)) {
			return Quantity * ItemData->NumericData.ItemWeight;
		}
		else {
			return -1.0f;
		}
	}

	UFUNCTION(Category = "Item")
	FORCEINLINE float GetItemWeight() const	// ������ ����
	{
		if (const FNAItemBaseTableRow* ItemData = GetItemMetaData<FNAItemBaseTableRow>(ItemDataTableRowHandle)) {
			return  ItemData->NumericData.ItemWeight;
		}
		else {
			return -1.0f;
		}
	}

	UFUNCTION(Category = "Item")
	FORCEINLINE bool IsFullItemStack() const  // ������ ������ ���� á���� ����
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
	virtual void UseItem(AActor* User);	// ������ ���

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
	/** ��ü�� ������ ������ ++ �Ͽ� ID �� �̾� �ִ� ������ ī���� */
	static FThreadSafeCounter IDCount;
	int32 IDNumber = -1;

public:
	UPROPERTY(VisibleAnywhere, Category = "Item Data", meta = (UIMin = 1, UIMax = 100))
	int32 Quantity = 1;	// �������� �ִ� ����

	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	FName ID;  // ������ ID (DT �� �̸� + ����)

	UPROPERTY(EditAnywhere, Category = "Item Data")
	EItemState ItemState = EItemState::IS_None;

	//UPROPERTY(VisibleAnywhere, Category = "Item Ownership")
	//TWeakObjectPtr<UInventoryComponent> OwningInventory = nullptr;	// ���� ���� �κ��丮

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
		ensureAlwaysMsgf(false, TEXT("[UNAItemData::CreateItemData]  RowName�� ��ȿ���� ����."));
		return nullptr;
	}

	if (!InItemMetaData) {
		ensureAlwaysMsgf(false, TEXT("[UNAItemData::CreateItemData]  InItemMetaData�� ��ȿ���� ����."));
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
			ensureAlwaysMsgf(false, TEXT("[UNAItemData::CreateItemData]  OuterWorld�� ��ȿ���� ����."));
			return nullptr;
		}
	}

	NewItemData = NewObject<UNAItemData>(NewOuter, NAME_None, RF_Transactional);
	if (!NewItemData) {
		ensureAlwaysMsgf(false, TEXT("[UNAItemData::CreateItemData]  NewItemData ���� ����."));
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
//		"[UNAItemData::CreateItemData]  ItemDataT�� �ݵ�� UNAItemData�� �Ļ� Ŭ�����̾�� �մϴ�.");*/
//
//	UClass* ItemDataClass = ItemDataT::StaticClass();
//	if (ItemDataClass != NewItemDataClass.Get()) {
//		UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::CreateItemData]  ItemDataClass�� NewItemDataClass�� �ٸ�. %s, %s"), *ItemDataClass->GetName(), *NewItemDataClass->GetName());
//		return nullptr;
//	}
//
//	if (!InDataTableRowHandle.IsNull()) {
//		bool bIsCompatibleDataTable = ItemDataClass->GetDefaultObject()->IsCompatibleDataTable(InDataTableRowHandle);
//		if (!bIsCompatibleDataTable) {
//			//UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::CreateItemData]  ������ Ŭ���� ���� DT�� ����.  %s, %s"), *ItemDataT::StaticClass()->GetName(), *InDataTableRowHandle.ToDebugString());
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
//		"[UNAItemData::CloneItemData]  ItemDataT�� �ݵ�� UNAItemData�� �Ļ� Ŭ�����̾�� �մϴ�.");
//
//	if (!InOriginal) {
//		UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::CloneItemData]  InOriginal�� �������� ����."));
//		return nullptr;
//	}
//
//	if (InOriginal->ItemDataTableRowHandle.IsNull()) {
//		UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::CloneItemData]  InOriginal�� ItemDataTableRowHandle�� ��ȿ���� ����."));
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
//		UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::CloneItemData]  NewItemData ������� ����. %s"), *InOriginal->GetClass()->GetName());
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
		UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::GetItemMetaData]  ������ ������ ���̺� �б� ����.  %s"), *InDataTableRowHandle.ToDebugString());
		return nullptr;
	}

	return ItemMetaData;
}