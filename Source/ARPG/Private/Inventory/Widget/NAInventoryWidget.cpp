// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Widget/NAInventoryWidget.h"

#include "Inventory/NAInventoryComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"

void UNAInventoryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	WeaponSlotButtons[0] = Weapon_00;
	WeaponSlotButtons[1] = Weapon_01;
	WeaponSlotButtons[2] = Weapon_02;
	WeaponSlotButtons[3] = Weapon_03;

	InvenSlotButtons[0][0] = Inven_00;
	InvenSlotButtons[0][1] = Inven_01;
	InvenSlotButtons[0][2] = Inven_02;
	InvenSlotButtons[0][3] = Inven_03;
	InvenSlotButtons[0][4] = Inven_04;

	InvenSlotButtons[1][0] = Inven_05;
	InvenSlotButtons[1][1] = Inven_06;
	InvenSlotButtons[1][2] = Inven_07;
	InvenSlotButtons[1][3] = Inven_08;
	InvenSlotButtons[1][4] = Inven_09;

	InvenSlotButtons[2][0] = Inven_10;
	InvenSlotButtons[2][1] = Inven_11;
	InvenSlotButtons[2][2] = Inven_12;
	InvenSlotButtons[2][3] = Inven_13;
	InvenSlotButtons[2][4] = Inven_14;

	InvenSlotButtons[3][0] = Inven_15;
	InvenSlotButtons[3][1] = Inven_16;
	InvenSlotButtons[3][2] = Inven_17;
	InvenSlotButtons[3][3] = Inven_18;
	InvenSlotButtons[3][4] = Inven_19;

	InvenSlotButtons[4][0] = Inven_20;
	InvenSlotButtons[4][1] = Inven_21;
	InvenSlotButtons[4][2] = Inven_22;
	InvenSlotButtons[4][3] = Inven_23;
	InvenSlotButtons[4][4] = Inven_24;
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
