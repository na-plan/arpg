// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Widget/NAInventoryWidget.h"

#include "Item/ItemData/NAItemData.h"
#include "Inventory/NAInventoryComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
//#include "Blueprint/WidgetNavigation.h"

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
	
	InitInvenSlotSlates();
	InitWeaponSlotSlates();

	InitInvenButtonsNavigation();
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
	if (bHaveInvenSlotsMapped) return;
	
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
	if (bHaveWeaponSlotsMapped) return;
	
	bool bResult = true;
	for (int32 i = 0; i < MaxWeaponSlots; ++i)
	{
		FString NumStr = FString::Printf(TEXT("%02d"), i);
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

// 인벤토리 슬롯(버튼) 네비게이션 설정
void UNAInventoryWidget::InitInvenButtonsNavigation() const
{
	if (!ensure(bHaveInvenSlotsMapped)) return;

	for (int Row = 0; Row < InventoryRowCount; ++Row)
	{
		for (int Col = 0; Col < InventoryColumnCount; ++Col)
		{
			UButton* CurButton = InvenSlotWidgets[Row][Col].InvenSlotButton.Get();
			if (!CurButton) continue;

			// 위쪽: 같은 열의 윗 행 버튼
			if (Row > 0)
			{
				UButton* NextButton = InvenSlotWidgets[Row-1][Col].InvenSlotButton.Get();
				if (!NextButton)
				{
					UE_LOG(LogTemp, Warning, TEXT("[InitInvenButtonsNavigation]  버튼 네비게이션 설정 실패: %d번, 행: %d, 열: %d")
						,1, Row - 1, Col);
					continue;
				}
				CurButton->SetNavigationRuleExplicit(EUINavigation::Up, NextButton);
			}
			// 아래쪽: 같은 열의 아랫 행 버튼
			if (Row < InventoryRowCount && Row + 1 < InventoryRowCount)
			{
				UButton* NextButton = InvenSlotWidgets[Row + 1][Col].InvenSlotButton.Get();
				if (!NextButton)
				{
					UE_LOG(LogTemp, Warning, TEXT("[InitInvenButtonsNavigation]  버튼 네비게이션 설정 실패: %d번, 행: %d, 열: %d")
						,2, Row + 1, Col);
					continue;
				}
				CurButton->SetNavigationRuleExplicit(EUINavigation::Down, NextButton);
			}
			// 왼쪽
			if (Col > 0)
				{
					UButton* NextButton = InvenSlotWidgets[Row][Col - 1].InvenSlotButton.Get();
					if (!NextButton)
					{
						UE_LOG(LogTemp, Warning, TEXT("[InitInvenButtonsNavigation]  버튼 네비게이션 설정 실패: %d번, 행: %d, 열: %d")
						       , 3, Row, Col - 1);
						continue;
					}
					CurButton->SetNavigationRuleExplicit(EUINavigation::Left, NextButton);
				}
			// 오른쪽
			if (Col < InventoryColumnCount && Col + 1 < InventoryColumnCount)
			{
				UButton* NextButton = InvenSlotWidgets[Row][Col + 1].InvenSlotButton.Get();
				if (!NextButton)
				{
					UE_LOG(LogTemp, Warning, TEXT("[InitInvenButtonsNavigation]  버튼 네비게이션 설정 실패: %d번, 행: %d, 열: %d")
						   , 4, Row, Col + 1);
					continue;
				}
				CurButton->SetNavigationRuleExplicit(EUINavigation::Right, NextButton);
			}
		}
	}
}

void UNAInventoryWidget::InitWeaponButtonsNavigation() const
{
	if (!ensure(bHaveWeaponSlotsMapped)) return;

	for (int Index = 0; Index < MaxWeaponSlots; ++Index)
	{
		UButton* CurButton = WeaponSlotWidgets[Index].WeaponSlotButton.Get();
		if (!CurButton) continue;

		if (Index == 0)
		{
			CurButton->SetNavigationRuleExplicit(EUINavigation::Up, WeaponSlotWidgets[1].WeaponSlotButton.Get());
			CurButton->SetNavigationRuleExplicit(EUINavigation::Down, WeaponSlotWidgets[1].WeaponSlotButton.Get());
			CurButton->SetNavigationRuleExplicit(EUINavigation::Left, WeaponSlotWidgets[2].WeaponSlotButton.Get());
			CurButton->SetNavigationRuleExplicit(EUINavigation::Right, WeaponSlotWidgets[3].WeaponSlotButton.Get());
		}
	}
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
	PlayAnimationReverse(WidgetExpand, 1.3f);
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
