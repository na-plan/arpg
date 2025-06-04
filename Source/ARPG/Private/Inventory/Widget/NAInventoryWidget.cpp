// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Widget/NAInventoryWidget.h"

#include "Inventory/NAInventoryComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"

void UNAInventoryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
}

bool UNAInventoryWidget::MapSlotIDAndUIButton(TMap<FName, TWeakObjectPtr<UButton>>& SlotButtons) const
{
	if (!ensure(SlotButtons.Num() == UNAInventoryComponent::MaxTotalSlots))
	{
		return false;
	}
	
	UClass* MyClass = GetClass();
	if (!MyClass) { return false; }

	int32 WeaponSlotCount = UNAInventoryComponent::MaxWeaponSlots;
	int32 InventorySlotCount = UNAInventoryComponent::MaxInventorySlots;

	for (TFieldIterator<FProperty> It(MyClass); It; ++It)
	{
		FProperty* Property = *It;
		if (!Property) { continue; }
		bool bIsWeaponSlot = false;
		bool bIsInventorySlot = false;
		if (!Property->HasMetaData("Category")) { continue; }

		FString Category = Property->GetMetaData("Category");
		if (Category.Equals(TEXT("Weapon Slots")))
		{
			bIsWeaponSlot = true;
		}
		else if (Category.Equals(TEXT("Inventory Slots")))
		{
			bIsInventorySlot = true;
		}
		else { continue; }
		
		FName PropName = FName(Property->GetName());
		if (!SlotButtons.Contains(PropName)) { continue; }

		if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Property))
		{
			UObject* Obj = ObjProp->GetObjectPropertyValue(this);
			if (UButton* SlotButton = Cast<UButton>(Obj))
			{
				SlotButtons[PropName] = SlotButton;
				if (bIsWeaponSlot)
				{
					WeaponSlotCount -= 1;
				}
				else if (bIsInventorySlot)
				{
					InventorySlotCount -= 1;
				}
			}
		}
	}

	ensureAlwaysMsgf(false, TEXT("[UNAInventoryWidget::MapSlotIDAndUIButton]  웨폰 슬롯 매핑 결과: %d개 실패"), WeaponSlotCount);
	ensureAlwaysMsgf(false, TEXT("[UNAInventoryWidget::MapSlotIDAndUIButton]  인벤 슬롯 매핑 결과: %d개 실패"), InventorySlotCount);
	return true;
}

void UNAInventoryWidget::RefreshWeaponSlotButtons(const TMap<FName, TWeakObjectPtr<UNAItemData>>& InventoryItems,
	const TMap<FName, TWeakObjectPtr<UButton>>& SlotButtons)
{
	const bool bValidCheck = InventoryItems.Num() == UNAInventoryComponent::MaxInventorySlots
			&& SlotButtons.Num() == UNAInventoryComponent::MaxWeaponSlots;
	if (!ensureAlwaysMsgf(bValidCheck,TEXT("[UNAInventoryWidget::RefreshWeaponSlotButtons]  InventoryItems.Num()과 SlotButtons.Num()이 유효하지 않음")))
	{
		return;
	}

	InvalidateLayoutAndVolatility();
}

void UNAInventoryWidget::RefreshInvenSlotButtons(const TMap<FName, TWeakObjectPtr<UNAItemData>>& InventoryItems,
	const TMap<FName, TWeakObjectPtr<UButton>>& SlotButtons)
{
	
}

void UNAInventoryWidget::RefreshSlotButton(const UNAItemData* ItemData, UButton* SlotButton)
{
	
}
