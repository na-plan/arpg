// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Widget/NAInventoryWidget.h"

#include "Item/ItemData/NAItemData.h"
#include "Inventory/NAInventoryComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"


FNAInvenSlotWidgets::FNAInvenSlotWidgets(UButton* Button, UImage* Icon, UTextBlock* Text)
	: InvenSlotButton(Button), InvenSlotIcon(Icon), InvenSlotQty(Text)
{
}

FNAWeaponSlotWidgets::FNAWeaponSlotWidgets(UButton* Button, UImage* Icon)
	: WeaponSlotButton(Button), WeaponSlotIcon(Icon)
{
}

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
	constexpr int32 NumCols = InventoryColumnCount;
	constexpr int32 MaxIndex = MaxInventorySlots - 1;
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
	
	//InitInvenSlotSlates();
	//bHaveWeaponSlotsMapped = true;
}

void UNAInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (OwningInventoryComponent)
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UNAInventoryWidget::InitInvenSlotSlates()
{
	bool bResult = true;
	for (int32 i = 0; i < MaxInventorySlots; ++i)
	{
		int32 Row = i / InventoryRowCount;
		int32 Col = i % InventoryColumnCount;

		FString NumStr = FString::Printf(TEXT("%02d"), i);
		FName BtnName = FName(*FString::Printf(TEXT("Inven_%s"), *NumStr));
		FName IconName = FName(*FString::Printf(TEXT("Inven_%s_Icon"), *NumStr));
		FName QtyName = FName(*FString::Printf(TEXT("Inven_%s_Qty"), *NumStr));

		UButton* Btn = Cast<UButton>(GetWidgetFromName(BtnName));
		UImage* Icon = Cast<UImage>(GetWidgetFromName(IconName));
		UTextBlock* Qty = Cast<UTextBlock>(GetWidgetFromName(QtyName));

		if (!Btn || !Icon || !Qty)
		{
			bResult = false;
			UE_LOG(LogTemp, Warning, TEXT("Inventory Slot Widget Binding Failed: [%02d] (%s%s%s)"),
				i,
				Btn ? TEXT("") : TEXT("Btn "),
				Icon ? TEXT("") : TEXT("Icon "),
				Qty ? TEXT("") : TEXT("Qty ")
			);
			continue;
		}
		
		InvenSlotWidgets[Row][Col] = FNAInvenSlotWidgets(Btn, Icon, Qty);
	}
	ensureAlwaysMsgf(bResult, TEXT("[InitInvenSlotSlates]  Failed to initialize inventory slots binding."));
	bHaveInvenSlotsMapped = bResult;
}

void UNAInventoryWidget::InitWeaponSlotSlates()
{
	bool bResult = true;
	for (int32 i = 0; i < MaxWeaponSlots; ++i)
	{
		FString NumStr = FString::FromInt(i);
		FName BtnName = FName(*FString::Printf(TEXT("Weapon_%s"), *NumStr));
		FName IconName = FName(*FString::Printf(TEXT("Weapon_%s_Icon"), *NumStr));

		UButton* Btn = Cast<UButton>(GetWidgetFromName(BtnName));
		UImage* Icon = Cast<UImage>(GetWidgetFromName(IconName));
		
		if (!Btn || !Icon)
		{
			bResult = false;
			UE_LOG(LogTemp, Warning, TEXT("Weapon Slot Widget Binding Failed: [%02d] (%s%s)"),
				i,
				Btn ? TEXT("") : TEXT("Btn "),
				Icon ? TEXT("") : TEXT("Icon ")
			);
			continue;
		}

		WeaponSlotWidgets[i] = FNAWeaponSlotWidgets(Btn, Icon);
	}
	ensureAlwaysMsgf(bResult, TEXT("[InitWeaponSlotSlates]  Failed to initialize weapon slots binding."));
	bHaveWeaponSlotsMapped = bResult;
}

// void UNAInventoryWidget::FillSlotButtonMapFromArrays(TMap<FName, TWeakObjectPtr<UButton>>& OutSlotButtons) const
// {
// 	// 1. Weapon 슬롯
// 	for (int i = 0; i < 4; ++i)
// 	{
// 		//FString SlotIDStr = FString::Printf(TEXT("Weapon_%02d"), i);
// 		FName SlotID = UNAInventoryComponent::MakeWeaponSlotID(i);
// 		OutSlotButtons.Add(SlotID, WeaponSlotButtons[i]);
// 	}
//
// 	// 2. 인벤 슬롯
// 	for (int row = 0; row < 5; ++row)
// 	{
// 		for (int col = 0; col < 5; ++col)
// 		{
// 			int index = row * 5 + col; // 0~24
// 			//FString SlotIDStr = FString::Printf(TEXT("Inven_%02d"), index);
// 			FName SlotID = UNAInventoryComponent::MakeInventorySlotID(index);
// 			OutSlotButtons.Add(SlotID, InvenSlotButtons[row][col]);
// 		}
// 	}
// }

void UNAInventoryWidget::ReleaseInventoryWidget()
{
	if (!OwningInventoryComponent || !WidgetExpand) return;
	
	bReleaseInventoryWidget = true;
	OwningInventoryComponent->SetVisibility(true);
	OwningInventoryComponent->SetWindowVisibility(EWindowVisibility::Visible);
	SetVisibility(ESlateVisibility::HitTestInvisible);
	PlayAnimationForward(WidgetExpand);
}

void UNAInventoryWidget::OnInventoryWidgetReleased()
{
	if (bReleaseInventoryWidget)
	{
		SetIsEnabled(true);
	}
}

void UNAInventoryWidget::CollapseInventoryWidget()
{
	if (!OwningInventoryComponent || !WidgetExpand) return;
	
	bReleaseInventoryWidget = false;
	SetIsEnabled(false);
	PlayAnimationReverse(WidgetExpand);
}

void UNAInventoryWidget::OnInventoryWidgetCollapsed()
{
	if (!bReleaseInventoryWidget)
	{
		if (OwningInventoryComponent)
		{
			SetVisibility(ESlateVisibility::Hidden);
			OwningInventoryComponent->SetWindowVisibility(EWindowVisibility::SelfHitTestInvisible);
			OwningInventoryComponent->SetVisibility(false);
		}
	}
}

UButton* UNAInventoryWidget::GetInvenSlotButton(const FName& SlotID) const
{
	ensure(bHaveInvenSlotsMapped);

	ExtractSlotNumber(SlotID);

	// @TODO
		
	return nullptr;
}

UButton* UNAInventoryWidget::GetWeaponSlotButton(const FName& SlotID) const
{
	ensure(bHaveWeaponSlotsMapped);
	
	ExtractSlotNumber(SlotID);

	// @TODO
	
	return nullptr;
}

void UNAInventoryWidget::RefreshSlotWidgets(const TMap<FName, TWeakObjectPtr<UNAItemData>>& InventoryItems
	, const TMap<FName, TWeakObjectPtr<UNAItemData>>& WeaponItems)
	/*, const TMap<FName, TWeakObjectPtr<UButton>>& SlotButtons)*/
{
	if (!ensure(InventoryItems.GetMaxIndex() == MaxInventorySlots
		&& WeaponItems.GetMaxIndex() == MaxWeaponSlots))
	{
		UE_LOG(LogTemp, Warning, TEXT("[RefreshSlotWidgets]  유효하지 않은 InventoryItems, WeaponItems"));
		return;
	}
	
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
			UE_LOG(LogTemp, Warning, TEXT("[RefreshWeaponSlotWidgets]  ConvertIndexTo2D: index -> [Row][Col] 변환실패."));
			continue;
		}

		FNAInvenSlotWidgets InvenSlotSlates = InvenSlotWidgets[Row][Col];
		check(InvenSlotSlates.IsValid());

		// 인벤토리 슬롯 UI 갱신 함수 호출
		UpdateInvenSlotDrawData(FoundItem, InvenSlotSlates);
	}

	for (const auto& Pair : WeaponItems)
	{
		const FName& SlotID = Pair.Key;
		// 해당 슬롯에 할당된 아이템 데이터 가져오기
		const TWeakObjectPtr<UNAItemData>* FoundItemPtr = InventoryItems.Find(SlotID);
		check(FoundItemPtr->IsValid());
		const UNAItemData* FoundItem = FoundItemPtr->Get();
		
		int32 Index = ExtractSlotNumber(SlotID);
		if (!FMath::IsWithinInclusive(Index, 0, MaxWeaponSlots))
		{
			UE_LOG(LogTemp, Warning, TEXT("[RefreshSlotWidgets]  슬롯ID의 뒷 넘버가 유효한 Index가 아니었음."));
			continue;
		}

		FNAWeaponSlotWidgets WeaponSlotSlates = WeaponSlotWidgets[Index];
		check(WeaponSlotSlates.IsValid());

		// 무기 슬롯 UI 갱신 함수 호출
		UpdateWeaponSlotDrawData(FoundItem, WeaponSlotSlates);
	}
	
	InvalidateLayoutAndVolatility();
}

// void UNAInventoryWidget::RefreshWeaponSlotWidgets(const TMap<FName, TWeakObjectPtr<UNAItemData>>& InventoryItems
// 	/*, const TMap<FName, TWeakObjectPtr<UButton>>& SlotButtons*/)
// {
// 	// 각 슬롯 ID 기준으로 순회
// 	for (const auto& Pair : InventoryItems)
// 	{
// 	}
// 	
// 	InvalidateLayoutAndVolatility();
// }

void UNAInventoryWidget::UpdateInvenSlotDrawData(const UNAItemData* ItemData, const FNAInvenSlotWidgets& Inven_SlotWidgets)
{
	// SlotButton의 이름에서 SlotID 추출
	//FName SlotID = Inven_SlotWidgets.InvenSlotButton->GetFName();

	// 1) 버튼, 아이콘, 수량 텍스트 찾기
	UButton* SlotButton = Inven_SlotWidgets.InvenSlotButton.Get();
	UImage* IconImage = Inven_SlotWidgets.InvenSlotIcon.Get();
	UTextBlock* NumText = Inven_SlotWidgets.InvenSlotQty.Get();
	
	// 2-A) ItemData가 있다면 내용 갱신
	if (ItemData)
	{
		// 아이템 Texture 및 수량
		UTexture2D* Icon = ItemData->GetItemIcon();
		int32 Quantity = ItemData->GetQuantity();
	
		if (IconImage && Icon)
		{
			IconImage->SetBrushFromTexture(Icon);
			IconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		if (NumText)
		{
			NumText->SetText(FText::AsNumber(Quantity));
			NumText->SetVisibility(Quantity > 1 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		}
	
		// 슬롯 활성화 등 UI Feedback도 가능
		SlotButton->SetIsEnabled(true);
	}
	// 2-B) 아이템이 없는 빈 슬롯이면 아이콘, 텍스트 감추기
	else
	{
		if (IconImage)
		{
			IconImage->SetBrushFromTexture(nullptr);
			IconImage->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (NumText)
		{
			NumText->SetText(FText::GetEmpty());
			NumText->SetVisibility(ESlateVisibility::Collapsed);
		}
		SlotButton->SetIsEnabled(false);
	}
}

void UNAInventoryWidget::UpdateWeaponSlotDrawData(const UNAItemData* ItemData, const FNAWeaponSlotWidgets& Weapon_SlotWidgets)
{
	// SlotButton의 이름에서 SlotID 추출
	//FName SlotID = Weapon_SlotWidgets.WeaponSlotButton->GetFName();
	
	// 1) 버튼, 아이콘, 수량 텍스트 찾기
	UButton* SlotButton = Weapon_SlotWidgets.WeaponSlotButton.Get();
	UImage* IconImage = Weapon_SlotWidgets.WeaponSlotIcon.Get();
	
	// 2-A) ItemData가 있다면 내용 갱신
	if (ItemData)
	{
		// 아이템 Texture 및 수량
		UTexture2D* Icon = ItemData->GetItemIcon();
		int32 Quantity = ItemData->GetQuantity();
	
		if (IconImage && Icon)
		{
			IconImage->SetBrushFromTexture(Icon);
			IconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	
		// 슬롯 활성화 등 UI Feedback도 가능
		SlotButton->SetIsEnabled(true);
	}
	// 2-B) 아이템이 없는 빈 슬롯이면 아이콘, 텍스트 감추기
	else
	{
		if (IconImage)
		{
			IconImage->SetBrushFromTexture(nullptr);
			IconImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	
		SlotButton->SetIsEnabled(false);
	}
}
