#include "Item/ItemDataStructs/NAItemBaseDataStructs.h"
#include "Item/ItemActor/NAWeapon.h"
#include "Item/EngineSubsystem/NAItemEngineSubsystem.h"

FNAItemBaseTableRow::FNAItemBaseTableRow(UClass* InItemClass)
{
	//UE_LOG(LogTemp, Warning, TEXT("[FNAItemBaseTableRow::FNAItemBaseTableRow]  아이템 DT 기본 생성자"));

	if (InItemClass && InItemClass->IsChildOf<ANAItemActor>())
	{
		ItemClass = InItemClass;
	}
}

#if WITH_EDITOR
template<typename EnumType>
FString EnumToDisplayString(const EnumType EnumeratorValue)
{
	// For the C++ enum.
	static_assert(TIsEnum<EnumType>::Value, "EnumeratorValue must be enum types.");
	const UEnum* EnumClass = StaticEnum<EnumType>();
	check(EnumClass != nullptr);
	return EnumClass->GetNameStringByValue(static_cast<int64>(EnumeratorValue));
}

FString InsertSpacesBeforeUppercaseSmart(const FString& Input)
{
	FString Result;
	const int32 Len = Input.Len();

	for (int32 i = 0; i < Len; ++i)
	{
		const TCHAR Char = Input[i];
		if (i == 0)
		{
			Result.AppendChar(Char);
		}
		else
		{
			bool bIsCurrentUpper = FChar::IsUpper(Char);
			bool bIsPrevUpper = FChar::IsUpper(Input[i - 1]);

			// 현재가 대문자고, 이전이 소문자인 경우에만 공백 추가
			if (bIsCurrentUpper && !bIsPrevUpper)
			{
				Result.AppendChar(TEXT(' '));
			}
			Result.AppendChar(Char);
		}
	}
	return Result;
}

void FNAItemBaseTableRow::OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName)
{
	FNAItemBaseTableRow* ItemRowStruct = InDataTable->FindRow<FNAItemBaseTableRow>(InRowName, TEXT("On Data Table Changed"));
	
	if (ItemRowStruct == this)
	{
		if (UNAItemEngineSubsystem::Get()
			&& UNAItemEngineSubsystem::Get()->IsItemMetaDataInitialized())
		{
			UClass* ItemActorClass = ItemRowStruct->ItemClass.Get();
			if (ensure(ItemActorClass))
			{
				if (!UNAItemEngineSubsystem::Get()->IsRegisteredItemMetaClass(ItemActorClass))
				{
					UNAItemEngineSubsystem::Get()->RegisterNewItemMetaData(ItemActorClass, InDataTable, InRowName);
				}
				else
				{
					UNAItemEngineSubsystem::Get()->VerifyItemMetaDataRowHandle(ItemActorClass, InDataTable, InRowName);
				}
			}
		}

		if (ItemType == EItemType::IT_Weapon
			|| ItemClass.Get()->IsChildOf<ANAWeapon>())
		{
			NumericData.bIsStackable = false;
			NumericData.MaxSlotStackSize = 1;
			NumericData.MaxInventoryHoldCount = 1;
		}
		else
		{
			if (!NumericData.bIsStackable)
			{
				NumericData.MaxSlotStackSize = 1;
				NumericData.MaxInventoryHoldCount = FMath::Max(0, NumericData.MaxInventoryHoldCount);
			}
			
			if (NumericData.MaxInventoryHoldCount == 0)
			{
				NumericData.MaxSlotStackSize = FMath::Max(0, NumericData.MaxSlotStackSize);
			}
			else if (NumericData.MaxInventoryHoldCount > 0)
			{
				NumericData.MaxSlotStackSize = FMath::Max(1, NumericData.MaxSlotStackSize);
			}
		}
		
		if (!InRowName.IsNone())
		{
			FString NewItemName = InsertSpacesBeforeUppercaseSmart(InRowName.ToString());
			TextData.Name = FText::FromString(NewItemName);
		}

		if (InteractableData.InteractableType != ENAInteractableType::None)
		{
			FString EnumStr = EnumToDisplayString(InteractableData.InteractableType);
			EnumStr = InsertSpacesBeforeUppercaseSmart(EnumStr);
			InteractableData.InteractionName = FText::FromString(EnumStr);
		}
	}
}
#endif