// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NAInventoryWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UNAItemData;

UCLASS()
class ARPG_API UNAInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// CreateWidget으로 위젯 인스턴스가 생성되고 나서 호출됨
	virtual void NativeOnInitialized() override;

	//bool MapSlotIDAndUIButton(TMap<FName, TWeakObjectPtr<UButton>>& SlotButtons) const;
	
	void FillSlotButtonMapFromArrays(TMap<FName, TWeakObjectPtr<UButton>>& OutSlotButtons) const;

	bool HaveInvenSlotButtonsMapped() const { return bHaveInvenSlotsMapped; }
	bool HaveWeaponSlotButtonsMapped() const { return bHaveWeaponSlotsMapped; }

	UButton* GetInvenSlotButton(FName SlotName) const;
	UButton* GetWeaponSlotButton(FName SlotName) const;

	void RefreshInvenSlotButtons(const TMap<FName, TWeakObjectPtr<UNAItemData>>& InventoryItems
		/*, const TMap<FName, TWeakObjectPtr<UButton>>& SlotButtons*/);
	void RefreshWeaponSlotButtons(const TMap<FName, TWeakObjectPtr<UNAItemData>>& InventoryItems
		/*, const TMap<FName, TWeakObjectPtr<UButton>>& SlotButtons*/);
	
	void UpdateSlotDrawData(const UNAItemData* ItemData, UButton* SlotButton);
	
protected:
	// Slot Buttons //////////////////////////////////////////////////////////////////////////////////////
	// 변수명: 슬롯 ID와 일치시켜야 함
	
	// Inven Slots //////////////////////////////////////////////////////////

	TWeakObjectPtr<UButton> InvenSlotButtons[5][5];
	TWeakObjectPtr<UImage> InvenSlotImages[5][5];
	TWeakObjectPtr<UTextBlock> InvenSlotNumbers[5][5];
	uint8 bHaveInvenSlotsMapped : 1;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_00;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_00_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_00_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_01;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_01_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_01_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_02;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_02_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_02_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_03;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_03_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_03_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_04;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_04_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_04_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_05;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_05_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_05_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_06;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_06_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_06_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_07;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_07_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_07_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_08;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_08_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_08_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_09;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_09_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_09_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_10;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_10_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_10_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_11;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_11_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_11_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_12;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_12_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_12_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_13;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_13_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_13_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_14;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_14_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_14_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_15;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_15_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_15_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_16;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_16_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_16_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_17;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_17_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_17_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_18;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_18_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_18_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_19;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_19_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_19_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_20;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_20_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_20_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_21;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_21_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_21_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_22;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_22_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_22_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_23;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_23_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_23_Qty;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UButton> Inven_24;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Inven_24_Icon;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UTextBlock> Inven_24_Qty;

	
	// Weapon Slots //////////////////////////////////////////////////

	TWeakObjectPtr<UButton> WeaponSlotButtons[4];
	uint8 bHaveWeaponSlotsMapped : 1;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Weapon Slots")
	TObjectPtr<UButton> Weapon_00;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Weapon_00_Icon;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Weapon Slots")
	TObjectPtr<UButton> Weapon_01;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Weapon_01_Icon;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Weapon Slots")
	TObjectPtr<UButton> Weapon_02;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Weapon_02_Icon;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Weapon Slots")
	TObjectPtr<UButton> Weapon_03;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inven Slots")
	TObjectPtr<UImage> Weapon_03_Icon;

};
