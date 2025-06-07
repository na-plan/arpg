// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Widget/NAInventoryWidget.h"

#include "Item/ItemData/NAItemData.h"
#include "Inventory/NAInventoryComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"

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
}

void UNAInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (OwningInventoryComponent)
	{
		ForceLayoutPrepass();
		SetVisibility(ESlateVisibility::Collapsed);
			
		InitInvenSlotSlates();
		InitWeaponSlotSlates();

		InitInvenButtonsNavigation();
		InitWeaponButtonsNavigation();
		if (GetOwningPlayer())
		{
			UWidgetBlueprintLibrary::SetInputMode_GameOnly(GetOwningPlayer(), false);
		}
	}
}

void UNAInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// if (bHaveInvenSlotsMapped)
	// {
	// 	for (int Row = 0; Row < InventoryRowCount; ++Row)
	// 	{
	// 		for (int Col = 0; Col < InventoryColumnCount; ++Col)
	// 		{
	// 			FNAInvenSlotWidgets InvenSlot = InvenSlotWidgets[Row][Col];
	// 			if (!InvenSlot.IsValid()) continue;
	// 			if (InvenSlot.InvenSlotButton->HasKeyboardFocus())
	// 			{
	// 				//UE_LOG(LogTemp, Warning, TEXT("인벤 슬롯 버튼 포커스: %s"),  *InvenSlots.InvenSlotButton->GetName());
	// 				// if (GEngine) {
	// 				// 	FString Log = FString::Printf(TEXT("인벤 슬롯 버튼 포커스: %s"), *InvenSlot.InvenSlotButton->GetName());
	// 				// 	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, *Log);
	// 				// }
	//
	// 				InvenSlot.InvenSlotButton->SetBackgroundColor(FocusedInvenSlotBackgroundColor);
	// 				InvenSlot.InvenSlotButton->SetColorAndOpacity(FLinearColor::White);
	// 			}
	// 			else
	// 			{
	// 				InvenSlot.InvenSlotButton->SetBackgroundColor(DefaultInvenSlotColor);
	// 				InvenSlot.InvenSlotButton->SetColorAndOpacity(DefaultInvenSlotColor);
	// 			}
	// 		}
	// 	}
	// }
}

TArray<FNAInvenSlotWidgets> UNAInventoryWidget::GetInvenSlotWidgets() const
{
	TArray<FNAInvenSlotWidgets> InvenSlots;
	if (!bHaveInvenSlotsMapped) return InvenSlots;

	InvenSlots.Reserve(MaxInventorySlots);
	for (int Row = 0; Row < InventoryRowCount; ++Row)
	{
		for (int Col = 0; Col < InventoryColumnCount; ++Col)
		{
			InvenSlots.Add(InvenSlotWidgets[Row][Col]);
		}
	}
	
	return InvenSlots;
}

void UNAInventoryWidget::InitInvenSlotSlates()
{
	if (bHaveInvenSlotsMapped) return;
	
	bool bResult = true;
	InvenSButtonMap.Reserve(MaxInventorySlots);
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
		if (Btn)
		{
			TSharedPtr<SButton> SBtn = StaticCastSharedPtr<SButton>(Btn->GetCachedWidget());
			if (!SBtn.IsValid())
			{
				// 필요하다면 TakeWidget() 시도
				SBtn = StaticCastSharedPtr<SButton>(Btn->TakeWidget().ToSharedPtr());
			}
			if (SBtn.IsValid())
			{
				// 매핑
				InvenSButtonMap.Add(SBtn, Btn);
			}
		}
	}
	ensureAlwaysMsgf(bResult, TEXT("[InitInvenSlotSlates]  Failed to initialize inventory slots binding."));
	bHaveInvenSlotsMapped = bResult;
}

TArray<FNAWeaponSlotWidgets> UNAInventoryWidget::GetWeaponSlotWidgets() const
{
	TArray<FNAWeaponSlotWidgets> WeaponSlots;
	if (!bHaveWeaponSlotsMapped) return WeaponSlots;

	WeaponSlots.Reserve(MaxWeaponSlots);
	for (int32 i = 0; i < MaxWeaponSlots; ++i)
	{
		WeaponSlots.Add(WeaponSlotWidgets[i]);
	}
	return WeaponSlots;
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
		if (Btn)
		{
			TSharedPtr<SButton> SBtn = StaticCastSharedPtr<SButton>(Btn->GetCachedWidget());
			if (!SBtn.IsValid())
			{
				// 필요하다면 TakeWidget() 시도
				SBtn = StaticCastSharedPtr<SButton>(Btn->TakeWidget().ToSharedPtr());
			}
			if (SBtn.IsValid())
			{
				// 매핑
				WeaponSButtonMap.Add(SBtn, Btn);
			}
		}
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
			if (Col == 0)
			{
				CurButton->SetNavigationRuleExplicit(EUINavigation::Left, Weapon_03);
			}
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

	// 1) 각 슬롯 인덱스에 대응되는 2D 좌표 (X: +1→우측, –1→좌측 / Y: +1→위, –1→아래)
	const FVector2D Coords[MaxWeaponSlots] = {
		/* 0: Weapon_00 */ FVector2D( 0.f, +1.f),
		/* 1: Weapon_01 */ FVector2D( 0.f, -1.f),
		/* 2: Weapon_02 */ FVector2D(-1.f,  0.f),
		/* 3: Weapon_03 */ FVector2D(+1.f,  0.f)
	};

	// 2) 네비게이션 방향별 벡터 매핑
	const TPair<EUINavigation, FVector2D> DirVecs[4] = {
		{ EUINavigation::Up,    FVector2D( 0.f, +1.f) },
		{ EUINavigation::Down,  FVector2D( 0.f, -1.f) },
		{ EUINavigation::Left,  FVector2D(-1.f,  0.f) },
		{ EUINavigation::Right, FVector2D(+1.f,  0.f) }
	};

	// 3) 배열 순회 → 이웃 좌표가 있으면 룰 설정
	for (int Index = 0; Index < MaxWeaponSlots; ++Index)
	{
		UButton* CurBtn = WeaponSlotWidgets[Index].WeaponSlotButton.Get();
		if (!CurBtn) continue;

		const FVector2D MyCoord = Coords[Index];

		for (int i = 0; i < 4; ++i)
		{
			const EUINavigation NavDir = DirVecs[i].Key;
			const FVector2D    Offset = DirVecs[i].Value;
			const FVector2D    TargetCoord = MyCoord + Offset;
			
			if (FMath::Abs( TargetCoord.X) == 2 ||  FMath::Abs(TargetCoord.Y) == 2)
			{
				if (TargetCoord.X > 0)
				{
					CurBtn->SetNavigationRuleExplicit(NavDir, Inven_00);
				}
				else if (TargetCoord.Y > 0)
				{
					/*CurBtn->SetNavigationRuleExplicit(NavDir, );*/
				}
				continue;
			}

			UButton* NextBtn = WeaponSlotWidgets[i].WeaponSlotButton.Get();
			if (NextBtn)
			{
				CurBtn->SetNavigationRuleExplicit(NavDir, NextBtn);
			}
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
	if (!GetOwningPlayer()) return;
	
	bReleaseInventoryWidget = true;
	OwningInventoryComponent->SetVisibility(true);
	OwningInventoryComponent->SetWindowVisibility(EWindowVisibility::Visible);
	SetVisibility(ESlateVisibility::HitTestInvisible);
	PlayAnimationForward(WidgetExpand);
}

void UNAInventoryWidget::OnInventoryWidgetReleased()
{
	if (!OwningInventoryComponent || !WidgetExpand) return;
	if (!GetOwningPlayer()) return;
	
	if (bReleaseInventoryWidget)
	{
		// 움직이면서 인벤토리 위젯 조작 가능은 한데, 진입할때 게임쪽 입력이 끊김 whyyyyyyyyy
		UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(GetOwningPlayer(), nullptr, EMouseLockMode::LockOnCapture, true, false);
		
		OwningInventoryComponent->SetWindowFocusable(true);
		SetIsEnabled(true);
		if (!LastFocusedSlotButton.IsValid())
		{
			Weapon_00->SetKeyboardFocus();	
		}
		else
		{
			LastFocusedSlotButton->SetKeyboardFocus();
		}
	}
}

void UNAInventoryWidget::CollapseInventoryWidget()
{
	if (!OwningInventoryComponent || !WidgetExpand) return;
	if (!GetOwningPlayer()) return;
	
	UWidgetBlueprintLibrary::SetInputMode_GameOnly(GetOwningPlayer(), false);
	
	bReleaseInventoryWidget = false;
	OwningInventoryComponent->SetWindowFocusable(false);
	SetIsEnabled(false);
	PlayAnimationReverse(WidgetExpand, 1.3f);
}

void UNAInventoryWidget::OnInventoryWidgetCollapsed()
{
	if (!OwningInventoryComponent || !WidgetExpand) return;
	if (!GetOwningPlayer()) return;
	
	if (!bReleaseInventoryWidget)
	{
		SetVisibility(ESlateVisibility::Hidden);
		OwningInventoryComponent->SetWindowVisibility(EWindowVisibility::SelfHitTestInvisible);
		OwningInventoryComponent->SetVisibility(false);
	}
}

FReply UNAInventoryWidget::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	return Super::NativeOnFocusReceived(InGeometry, InFocusEvent).Handled();
}

void UNAInventoryWidget::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusLost(InFocusEvent);
}

void UNAInventoryWidget::NativeOnFocusChanging(const FWeakWidgetPath& PreviousFocusPath,
                                               const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusChanging(PreviousFocusPath, NewWidgetPath, InFocusEvent);

	TWeakPtr<SWidget> PreWidget = PreviousFocusPath.GetLastWidget();
	if (!PreWidget.IsValid()) return;

	TSharedPtr<SWidget> PreSWidgetRef = PreviousFocusPath.GetLastWidget().Pin();
	if (!PreSWidgetRef.IsValid()) return;

	// SButton으로 캐스팅
	TSharedPtr<SButton> PreSButtonWidget = StaticCastSharedPtr<SButton>(PreSWidgetRef);
	if (!PreSButtonWidget.IsValid()) return;

	// 키를 TWeakPtr로 만들어서 검색
	TWeakPtr<SButton> PreSKey = PreSButtonWidget;

	TWeakObjectPtr<UButton> PreUBtnPtr;
	if (InvenSButtonMap.Contains(PreSKey))
	{
		PreUBtnPtr = InvenSButtonMap[PreSKey];
		if (PreUBtnPtr.IsValid())
		{
			UButton* Btn = PreUBtnPtr.Get();
			Btn->SetBackgroundColor(DefaultInvenSlotColor);
			Btn->SetColorAndOpacity(DefaultInvenSlotColor);
			LastFocusedSlotButton = Btn; 
		}
	}
	else if (WeaponSButtonMap.Contains(PreSKey))
	{
		PreUBtnPtr = WeaponSButtonMap[PreSKey];
		if (PreUBtnPtr.IsValid())
		{
			UButton* Btn = PreUBtnPtr.Get();
			Btn->SetBackgroundColor(DefaultWeaponSlotColor);
			Btn->SetColorAndOpacity(DefaultWeaponSlotColor);
		}
	}
	
	TWeakPtr<SWidget> NewSWidget = NewWidgetPath.GetLastWidget();
	if (!NewSWidget.IsValid()) return;

	TSharedPtr<SWidget> NewSWidgetRef = NewWidgetPath.GetLastWidget();
	if (!NewSWidgetRef.IsValid()) return;

	// SButton으로 캐스팅
	TSharedPtr<SButton> NewSButtonWidget = StaticCastSharedPtr<SButton>(NewSWidgetRef);
	if (!NewSButtonWidget.IsValid()) return;

	// 키를 TWeakPtr로 만들어서 검색
	TWeakPtr<SButton> NewSKey = NewSButtonWidget;

	TWeakObjectPtr<UButton> NewUBtnPtr;
	if (InvenSButtonMap.Contains(NewSKey))
	{
		NewUBtnPtr = InvenSButtonMap[NewSKey];
		if (NewUBtnPtr.IsValid())
		{
			UButton* Btn = NewUBtnPtr.Get();
			Btn->SetBackgroundColor(FocusedInvenSlotBackgroundColor);
			Btn->SetColorAndOpacity(FLinearColor::White);
		}
	}
	else if (WeaponSButtonMap.Contains(NewSKey))
	{
		NewUBtnPtr = WeaponSButtonMap[NewSKey];
		if (NewUBtnPtr.IsValid())
		{
			UButton* Btn = NewUBtnPtr.Get();
			Btn->SetBackgroundColor(FocusedWeaponSlotBackgroundColor);
			Btn->SetColorAndOpacity(FLinearColor::White);
		}
	}
}

FReply UNAInventoryWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Up || Key == EKeys::Down || Key == EKeys::Left || Key == EKeys::Right)
	{
		return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent).Unhandled();
}

FReply UNAInventoryWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::Up || Key == EKeys::Down || Key == EKeys::Left || Key == EKeys::Right)
	{
		return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent).Unhandled();
}

FReply UNAInventoryWidget::NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::Up || Key == EKeys::Down || Key == EKeys::Left || Key == EKeys::Right)
	{
		return Super::NativeOnKeyUp(InGeometry, InKeyEvent);
	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent).Unhandled();
}

FReply UNAInventoryWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent).Unhandled();
}

FReply UNAInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent).Unhandled();
}

FReply UNAInventoryWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent).Unhandled();
}

FReply UNAInventoryWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent).Unhandled();
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
