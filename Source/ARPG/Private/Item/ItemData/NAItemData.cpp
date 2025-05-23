#include "Item/ItemData/NAItemData.h"
#include "Item/ItemActor/NAItemActor.h"
#include "Inventory/GameInstance/NAInventoryGameInstanceSubsystem.h"

// 프로그램 시작 시 0 에서 시작
FThreadSafeCounter UNAItemData::IDCount(0);

UNAItemData::UNAItemData()
{
	if (!HasAnyFlags(RF_ClassDefaultObject)) {
		IDNumber = IDCount.Increment();
	}
}

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

void UNAItemData::SetQuantity(const int32 NewQuantity)
{
	if (NewQuantity != Quantity)
	{
		if (const FNAItemBaseTableRow* ItemMetaData = GetItemMetaDataStruct<FNAItemBaseTableRow>()) {
			Quantity = FMath::Clamp(NewQuantity, 0, ItemMetaData->NumericData.bIsStackable ? ItemMetaData->NumericData.MaxSlotStackSize : 1);
			/*if (OwningInventory.Get())
			{
				if (Quantity <= 0)
				{
					OwningInventory->RemoveItem(this);
				}
			}*/
		}
	}
}

UClass* UNAItemData::GetItemActorClass() const
{
	UClass* ItemActorClass = nullptr;
	if (const FNAItemBaseTableRow* ItemMetaData = GetItemMetaDataStruct())
	{
		ItemActorClass = ItemMetaData->ItemClass.Get();
	}
	return ItemActorClass;
}
