// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Widget/NAInventoryWidget.h"

#include "Item/ItemData/NAItemData.h"
#include "Inventory/NAInventoryComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"


// 슬롯 ID에서 번호 추출 헬퍼 함수
// @param	SlotID: xxx_00 형식을 전제
int32 ExtractSlotNumber(const FName& SlotID)
{
	const FString SlotStr = SlotID.ToString();
	int32 UnderscoreIndex = INDEX_NONE;
	if (SlotStr.FindLastChar(TEXT('_'), UnderscoreIndex))
	{
		const FString NumberStr = SlotStr.Mid(UnderscoreIndex + 1);
		if (NumberStr.IsNumeric())
		{
			return FCString::Atoi(*NumberStr);
		}
		else
		{
			// 숫자가 아님: 예외 케이스
			ensureAlwaysMsgf(false, TEXT("[ExtractSlotNumber] 잘못된 SlotID(숫자 아님): %s"), *SlotStr);
			return INT_MAX;
		}
	}
	else
	{
		// 언더바 없음: 예외 케이스
		ensureAlwaysMsgf(false, TEXT("[ExtractSlotNumber] 언더바 없는 SlotID: %s"), *SlotStr);
		return INT_MAX;
	}
}

bool ConvertIndexTo2D(int32 Index, int32& OutRow, int32& OutCol)
{
	constexpr int32 NumCols = 5;
	constexpr int32 MaxIndex = 24;
	if (!ensureAlwaysMsgf(Index >= 0 && Index <= MaxIndex, TEXT("Index out of range: %d"), Index))
	{
		return false;
	}
	OutRow = Index / NumCols;
	OutCol = Index % NumCols;
	return true;
}

void UNAInventoryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
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
	bHaveInvenSlotsMapped = true;

	
	WeaponSlotButtons[0] = Weapon_00;
	WeaponSlotButtons[1] = Weapon_01;
	WeaponSlotButtons[2] = Weapon_02;
	WeaponSlotButtons[3] = Weapon_03;
	bHaveWeaponSlotsMapped = true;
}

void UNAInventoryWidget::FillSlotButtonMapFromArrays(TMap<FName, TWeakObjectPtr<UButton>>& OutSlotButtons) const
{
	// 1. Weapon 슬롯
	for (int i = 0; i < 4; ++i)
	{
		//FString SlotIDStr = FString::Printf(TEXT("Weapon_%02d"), i);
		FName SlotID = UNAInventoryComponent::MakeWeaponSlotID(i);
		OutSlotButtons.Add(SlotID, WeaponSlotButtons[i]);
	}

	// 2. 인벤 슬롯
	for (int row = 0; row < 5; ++row)
	{
		for (int col = 0; col < 5; ++col)
		{
			int index = row * 5 + col; // 0~24
			//FString SlotIDStr = FString::Printf(TEXT("Inven_%02d"), index);
			FName SlotID = UNAInventoryComponent::MakeInventorySlotID(index);
			OutSlotButtons.Add(SlotID, InvenSlotButtons[row][col]);
		}
	}
}

// bool UNAInventoryWidget::MapSlotIDAndUIButton(TMap<FName, TWeakObjectPtr<UButton>>& SlotButtons) const
// {
// 	if (!ensure(SlotButtons.Num() == UNAInventoryComponent::MaxTotalSlots))
// 	{
// 		return false;
// 	}
// 	
// 	UClass* MyClass = GetClass();
// 	if (!MyClass) { return false; }
//
// 	int32 WeaponSlotCount = UNAInventoryComponent::MaxWeaponSlots;
// 	int32 InventorySlotCount = UNAInventoryComponent::MaxInventorySlots;
//
// 	for (TFieldIterator<FProperty> It(MyClass); It; ++It)
// 	{
// 		FProperty* Property = *It;
// 		if (!Property) { continue; }
// 		bool bIsWeaponSlot = false;
// 		bool bIsInventorySlot = false;
// 		if (!Property->HasMetaData("Category")) { continue; }
//
// 		FString Category = Property->GetMetaData("Category");
// 		if (Category.Equals(TEXT("Weapon Slots")))
// 		{
// 			bIsWeaponSlot = true;
// 		}
// 		else if (Category.Equals(TEXT("Inventory Slots")))
// 		{
// 			bIsInventorySlot = true;
// 		}
// 		else { continue; }
// 		
// 		FName PropName = FName(Property->GetName());
// 		if (!SlotButtons.Contains(PropName)) { continue; }
//
// 		if (FObjectProperty* ObjProp = CastField<FObjectProperty>(Property))
// 		{
// 			UObject* Obj = ObjProp->GetObjectPropertyValue(this);
// 			if (UButton* SlotButton = Cast<UButton>(Obj))
// 			{
// 				SlotButtons[PropName] = SlotButton;
// 				if (bIsWeaponSlot)
// 				{
// 					WeaponSlotCount -= 1;
// 				}
// 				else if (bIsInventorySlot)
// 				{
// 					InventorySlotCount -= 1;
// 				}
// 			}
// 		}
// 	}
//
// 	ensureAlwaysMsgf(false, TEXT("[UNAInventoryWidget::MapSlotIDAndUIButton]  웨폰 슬롯 매핑 결과: %d개 실패"), WeaponSlotCount);
// 	ensureAlwaysMsgf(false, TEXT("[UNAInventoryWidget::MapSlotIDAndUIButton]  인벤 슬롯 매핑 결과: %d개 실패"), InventorySlotCount);
// 	return true;
// }

void UNAInventoryWidget::RefreshWeaponSlotButtons(const TMap<FName, TWeakObjectPtr<UNAItemData>>& InventoryItems
	/*, const TMap<FName, TWeakObjectPtr<UButton>>& SlotButtons*/)
{
	
	InvalidateLayoutAndVolatility();
}

UButton* UNAInventoryWidget::GetInvenSlotButton(FName SlotName) const
{
	ensure(bHaveInvenSlotsMapped);

		
	return nullptr;
}

UButton* UNAInventoryWidget::GetWeaponSlotButton(FName SlotName) const
{
	ensure(bHaveWeaponSlotsMapped);
	
	ExtractSlotNumber(SlotName);
	
	return nullptr;
}

void UNAInventoryWidget::RefreshInvenSlotButtons(const TMap<FName, TWeakObjectPtr<UNAItemData>>& InventoryItems
	/*, const TMap<FName, TWeakObjectPtr<UButton>>& SlotButtons*/)
{
	// 각 슬롯 ID 기준으로 순회
	for (const auto& Pair : InventoryItems)
	{
		
		const FName& SlotID = Pair.Key;
		// 해당 슬롯에 할당된 아이템 데이터 가져오기
		const TWeakObjectPtr<UNAItemData>* FoundItemPtr = InventoryItems.Find(SlotID);
		check(FoundItemPtr->IsValid());
		const UNAItemData* FoundItem = FoundItemPtr->Get();
		
		int32 Index = ExtractSlotNumber(SlotID);
		int32 Row = -1;
		int32 Col = -1;
		const bool bSucceed = ConvertIndexTo2D(Index, Row, Col);
		if (!bSucceed)
		{
			UE_LOG(LogTemp, Warning, TEXT("[RefreshWeaponSlotButtons]  ConvertIndexTo2D: index -> [Row][Col] 변환실패."));
			continue;
		}
		
		check(InvenSlotButtons[Row][Col].IsValid());
		UButton* SlotButton = InvenSlotButtons[Row][Col].Get();
		//SlotButton->child
		
		UNAItemData* ItemData = FoundItemPtr ? FoundItemPtr->Get() : nullptr;

		// 슬롯 UI 갱신 함수 호출
		UpdateSlotDrawData(ItemData, SlotButton);
	}
	
}

void UNAInventoryWidget::UpdateSlotDrawData(const UNAItemData* ItemData, UButton* SlotButton)
{
	// // 예시: SlotButton의 이름에서 SlotID 추출
	// FName SlotID = SlotButton->GetFName();
	//
	// // 1. 아이콘, 수량 텍스트 찾기 (이미 선언되어 있다면 switch/case로 직접 참조)
	// UImage* IconImage = nullptr;
	// UTextBlock* NumText = nullptr;
	// // (실제 구현에서는 SlotID마다 직접 멤버 변수로 접근 권장)
	// // ex: if (SlotID == "Inven_00") { IconImage = Inven_00_Icon; NumText = Inven_00_Num; }
	//
	// // 2. ItemData가 있다면 내용 갱신
	// if (ItemData)
	// {
	// 	// 아이템 Texture 및 수량
	// 	UTexture2D* Icon = ItemData->GetItemIcon();
	// 	int32 Quantity = ItemData->GetQuantity();
	//
	// 	if (IconImage && Icon)
	// 	{
	// 		IconImage->SetBrushFromTexture(Icon);
	// 		IconImage->SetVisibility(ESlateVisibility::Visible);
	// 	}
	// 	if (NumText)
	// 	{
	// 		NumText->SetText(FText::AsNumber(Quantity));
	// 		NumText->SetVisibility(Quantity > 1 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	// 	}
	//
	// 	// 슬롯 활성화 등 UI Feedback도 가능
	// 	SlotButton->SetIsEnabled(true);
	// }
	// else
	// {
	// 	// 아이템이 없는 빈 슬롯이면 아이콘, 텍스트 감추기
	// 	if (IconImage)
	// 	{
	// 		IconImage->SetBrushFromTexture(nullptr);
	// 		IconImage->SetVisibility(ESlateVisibility::Collapsed);
	// 	}
	// 	if (NumText)
	// 	{
	// 		NumText->SetText(FText::GetEmpty());
	// 		NumText->SetVisibility(ESlateVisibility::Collapsed);
	// 	}
	// 	SlotButton->SetIsEnabled(false);
	// }
}

