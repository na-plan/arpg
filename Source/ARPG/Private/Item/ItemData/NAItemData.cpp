
#include "Item/ItemData/NAItemData.h"

#include "Item/GameInstance/NAItemGameInstanceSubsystem.h"

// 프로그램 시작 시 0 에서 시작
FThreadSafeCounter UNAItemData::IDCount(0);

UNAItemData::UNAItemData()
{
	if (!HasAnyFlags(RF_ClassDefaultObject)) {
		IDNumber = IDCount.Increment();
	}
}

//UNAItemData::UNAItemData(const FObjectInitializer& ObjectInitializer)
//	: Super(ObjectInitializer)
//{
//	if (HasAnyFlags(RF_ClassDefaultObject)) {
//		static ConstructorHelpers::FObjectFinder<UDataTable> DefaultDT(TEXT("'"));
//		check(DefaultDT.Object);
//
//		ItemDataTableRowHandle.DataTable = DefaultDT.Object;
//		ItemDataTableRowHandle.RowName = TEXT("ItemDataCDO");
//	}
//}

//FDataTableRowHandle UNAItemData::GetDefaultRowHandle()
//{
//	const UNAItemData* DefaultObj = CastChecked<UNAItemData>(GetClass()->GetDefaultObject());
//	return DefaultObj->ItemDataTableRowHandle;
//}

void UNAItemData::PostInitProperties()
{
	Super::PostInitProperties();

	if (!HasAnyFlags(RF_ClassDefaultObject)) {
		if (UWorld* World = GetWorld()) {
			if (UGameInstance* GI = World->GetGameInstance()) {
				if (UNAItemGameInstanceSubsystem* ItemSubsys = GI->GetSubsystem<UNAItemGameInstanceSubsystem>()) {
					if (ID.IsNone()) {
						UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::PostInitProperties]  UNAItemData[%s]의 ID가 초기화되지 않았음. 어째서야"), *GetName());
					}
					if (GetOuter() != ItemSubsys) {
						UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::PostInitProperties]  UNAItemData[%s]의 Outer[%s]가 UNAItemGameInstanceSubsystem이 아니었음. 어째서야 "), *GetName(), *GetOuter()->GetName());
					}
				}
			}
		}
	}
	// sh1t
}

//UNAItemData* UNAItemData::CloneItemData(UObject* Outer, UNAItemData* InOriginal, UClass* InOriginalClass, const FDataTableRowHandle& InDataTableRowHandle)
//{
//	if (!InOriginal) {
//		UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::CloneItemData]  InOriginal이 유요하지 않음."));
//		return nullptr;
//	}
//
//	if (!InOriginalClass || !InOriginalClass->IsChildOf<UNAItemData>()) {
//		UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::CloneItemData]  InOriginalClass가 유요하지 않거나 UNAItemData의 파생 클래스가 아님."));
//		return nullptr;
//	}
//
//	UNAItemData* NewItemData = nullptr;
//	if (Outer) {
//		NewItemData = NewObject<UNAItemData>(Outer, InOriginalClass, NAME_None, RF_Transactional);
//	} 
//	else {
//		ULevel* OuterLevel = GetWorld()->GetCurrentLevel();
//		UObject* NewOuter = OuterLevel ? OuterLevel : (UObject*)GetTransientPackage();
//		NewItemData = NewObject<UNAItemData>(NewOuter, InOriginalClass, NAME_None, RF_Transactional);
//	}
//
//	if (NewItemData) {
//		if (InDataTableRowHandle.IsNull()) {
//			UE_LOG(LogTemp, Warning, TEXT("[UNAItemData::CloneItemData]  NewItemData은 생성했으나, InDataTableRowHandle이 Null이었음."));
//		}
//		else {
//			NewItemData->ItemDataTableRowHandle = InDataTableRowHandle;
//		}
//	}
//
//	return NewItemData;
//}

void UNAItemData::SetQuantity(const int32 NewQuantity)
{
	//if (NewQuantity != Quantity)
	//{
	//	if (const FNAItemBaseTableRow* ItemData = GetItemMetaData<FNAItemBaseTableRow>(ItemDataTableRowHandle)) {
	//		Quantity = FMath::Clamp(NewQuantity, 0, ItemData->NumericData.bIsStackable ? ItemData->NumericData.MaxStackSize : 1);
	//		/*if (OwningInventory.Get())
	//		{
	//			if (Quantity <= 0)
	//			{
	//				OwningInventory->RemoveItem(this);
	//			}
	//		}*/
	//	}
	//}
}

void UNAItemData::UseItem(AActor* User)
{
}

//bool UNAItemData::LoadItemData(const FNAItemBaseTableRow& InDataTableRow)
//{
//	return false;
//}

bool UNAItemData::IsCompatibleDataTable(const FDataTableRowHandle& InDataTableRowHandle) const
{
	if (InDataTableRowHandle.IsNull()) { return false; }

	const UDataTable* DT = InDataTableRowHandle.DataTable;
	if (!DT) { return false; }

	const UScriptStruct* RowStruct = DT->GetRowStruct();
	if (!RowStruct) { return false; }

	bool bIsCompatible = RowStruct == FNAItemBaseTableRow::StaticStruct();
	return bIsCompatible;
}