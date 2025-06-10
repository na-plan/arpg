// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Widget/NAInventoryWidget.h"

#include "Item/ItemData/NAItemData.h"
#include "Inventory/NAInventoryComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Widgets/SViewport.h"

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

bool ConvertIndexToInvenGrid(int32 Index, int32& OutRow, int32& OutCol)
{
	constexpr int32 NumCols = InventoryColumnCount;
	constexpr int32 MaxIndex = MaxInventorySlotCount - 1;
	if (!ensureAlwaysMsgf(Index >= 0 && Index <= MaxIndex, TEXT("Index out of range: %d"), Index))
	{
		return false;
	}
	OutRow = Index / NumCols;
	OutCol = Index % NumCols;
	return true;
}

bool UNAInventoryWidget::ParseSlotIDToInvenGrid(const FName& SlotID, int32& OutRow, int32& OutCol) const
{
	if (SlotID.IsNone()) return false;
	
	FString SlotStr = SlotID.ToString();
	if (SlotStr.StartsWith(InventorySlotPrefix))
	{
		int32 SlotNumber = ExtractSlotNumber(SlotID);
		if (SlotNumber == INT_MAX) return false;

		return ConvertIndexToInvenGrid(SlotNumber, OutRow, OutCol);
	}
	
	return false;
}

bool UNAInventoryWidget::ParseSlotIDToWeaponIndex(const FName& SlotID, int32& OutIndex) const
{
	if (SlotID.IsNone()) return false;
	FString SlotStr = SlotID.ToString();
	if (SlotStr.StartsWith(WeaponSlotPrefix))
	{
		OutIndex = ExtractSlotNumber(SlotID);
		return OutIndex != INT_MAX;
	}
	
	return false;
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
	}
}

void UNAInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

TArray<FNAInvenSlotWidgets> UNAInventoryWidget::GetInvenSlotWidgets() const
{
	TArray<FNAInvenSlotWidgets> InvenSlots;
	if (!bHaveInvenSlotsMapped) return InvenSlots;

	InvenSlots.Reserve(MaxInventorySlotCount);
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
	InvenSButtonMap.Reserve(MaxInventorySlotCount);
	for (int32 i = 0; i < MaxInventorySlotCount; ++i)
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

	WeaponSlots.Reserve(MaxWeaponSlotCount);
	for (int32 i = 0; i < MaxWeaponSlotCount; ++i)
	{
		WeaponSlots.Add(WeaponSlotWidgets[i]);
	}
	return WeaponSlots;
}

void UNAInventoryWidget::InitWeaponSlotSlates()
{
	if (bHaveWeaponSlotsMapped) return;
	
	bool bResult = true;
	for (int32 i = 0; i < MaxWeaponSlotCount; ++i)
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
	const FVector2D Coords[MaxWeaponSlotCount] = {
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
	for (int Index = 0; Index < MaxWeaponSlotCount; ++Index)
	{
		UButton* CurBtn = WeaponSlotWidgets[Index].WeaponSlotButton.Get();
		if (!CurBtn) continue;

		const FVector2D MyCoord = Coords[Index];

		for (int i = 0; i < 4; ++i)
		{
			const EUINavigation NavDir		 = DirVecs[i].Key;
			const FVector2D		Offset		 = DirVecs[i].Value;
			const FVector2D		TargetCoord	 = MyCoord + Offset;
			
			if (FMath::Abs( TargetCoord.X) == 2 ||  FMath::Abs(TargetCoord.Y) == 2)
			{
				if (TargetCoord.X > 0)
				{
					ensure(InvenSlotWidgets[2][0].IsValid());
					CurBtn->SetNavigationRuleExplicit(NavDir, InvenSlotWidgets[2][0].InvenSlotButton.Get());
				}
				else if (TargetCoord.Y > 0)
				{
					// 인벱토리 위젯 최상단 아이콘 버튼으로
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

FName UNAInventoryWidget::GetSlotID(const UButton* Button) const
{
	if (!Button) return NAME_None;
	
	// TArray<FNAInvenSlotWidgets> InvenSlotWidgetsArr = GetInvenSlotWidgets();
	// if (InvenSlotWidgetsArr.Num() != MaxInventorySlotCount) return NAME_None;
	//
	// for (int i = 0; i < InvenSlotWidgetsArr.Num(); ++i)
	// {
	// 	FNAInvenSlotWidgets SlotWidgets = InvenSlotWidgetsArr[i];
	// 	if (!SlotWidgets.IsValid()) continue;
	//
	// 	if (SlotWidgets.InvenSlotButton.Get() == Button)
	// 	{
	// 		return UNAInventoryComponent::MakeInventorySlotID(i);
	// 	}
	// }
	//
	// for (int i = 0; i < MaxWeaponSlotCount; ++i)
	// {
	// 	FNAWeaponSlotWidgets SlotWidgets = WeaponSlotWidgets[i];
	// 	if (!SlotWidgets.IsValid()) continue;
	//
	// 	if (SlotWidgets.WeaponSlotButton.Get() == Button)
	// 	{
	// 		return UNAInventoryComponent::MakeWeaponSlotID(i);
	// 	}
	// }

	FString SlotNameStr = Button->GetName();
	if (!SlotNameStr.StartsWith(InventorySlotPrefix) && !SlotNameStr.StartsWith(WeaponSlotPrefix))
		return NAME_None;

	return FName(*SlotNameStr);
}

FName UNAInventoryWidget::GetCurrentFocusedSlotID() const
{
	if (!CurrentFocusedSlotButton.IsValid()) return NAME_None;

	return GetSlotID(CurrentFocusedSlotButton.Get());
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
		UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(GetOwningPlayer(), nullptr, EMouseLockMode::LockAlways, true, false);

		// 일단 땜빵: [인벤 위젯 활성화 시 마우스 커서 노출되는 문제 + 마우스 버튼 다운/업 시 인벤 위젯에서 키보드 포커스 빠지는 문제] 틀어막음 
		// UGameViewportClient* GameViewportClient = GetWorld()->GetGameViewport();
		// TSharedPtr<SViewport> ViewportWidget = GameViewportClient->GetGameViewportWidget();
		// if (ViewportWidget.IsValid())
		// {
		// 	FReply& SlateOperations = GetOwningLocalPlayer()->GetSlateOperations();
		// 	TSharedRef<SViewport> ViewportWidgetRef = ViewportWidget.ToSharedRef();
		// 	SlateOperations.UseHighPrecisionMouseMovement(ViewportWidgetRef);
		// 	SlateOperations.LockMouseToWidget(ViewportWidgetRef);
		// 	GameViewportClient->SetIgnoreInput(false);
		// 	GameViewportClient->SetHideCursorDuringCapture(true);
		// 	GameViewportClient->SetMouseCaptureMode(EMouseCaptureMode::NoCapture);
		// }
		
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
	bReleaseInventoryWidget = false;
	
	UWidgetBlueprintLibrary::SetInputMode_GameOnly(GetOwningPlayer(), false);
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

	bool bIsLastFocusedBtnValid = false;
	bool bIsCurrentFocusedBtnValid = false;
	
	TWeakPtr<SWidget> PreWidget = PreviousFocusPath.GetLastWidget();
	if (!PreWidget.IsValid())
	{
		//LastFocusedSlotButton = bIsLastFocusedBtnValid ? LastFocusedSlotButton : nullptr;
		//CurrentFocusedSlotButton = bIsCurrentFocusedBtnValid ? CurrentFocusedSlotButton : nullptr;
		return;
	}

	TSharedPtr<SWidget> PreSWidgetRef = PreviousFocusPath.GetLastWidget().Pin();
	if (!PreSWidgetRef.IsValid())
	{
		//LastFocusedSlotButton = bIsLastFocusedBtnValid ? LastFocusedSlotButton : nullptr;
		//CurrentFocusedSlotButton = bIsCurrentFocusedBtnValid ? CurrentFocusedSlotButton : nullptr;
		return;
	}	

	// SButton으로 캐스팅
	TSharedPtr<SButton> PreSButtonWidget = StaticCastSharedPtr<SButton>(PreSWidgetRef);
	if (!PreSButtonWidget.IsValid())
	{
		//LastFocusedSlotButton = bIsLastFocusedBtnValid ? LastFocusedSlotButton : nullptr;
		//CurrentFocusedSlotButton = bIsCurrentFocusedBtnValid ? CurrentFocusedSlotButton : nullptr;
		return;
	}

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
			bIsLastFocusedBtnValid = true;
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
			LastFocusedSlotButton = Btn;
			bIsLastFocusedBtnValid = true;
		}
	}
	
	TWeakPtr<SWidget> NewSWidget = NewWidgetPath.GetLastWidget();
	if (!NewSWidget.IsValid())
	{
		//LastFocusedSlotButton = bIsLastFocusedBtnValid ? LastFocusedSlotButton : nullptr;
		//CurrentFocusedSlotButton = bIsCurrentFocusedBtnValid ? CurrentFocusedSlotButton : nullptr;
		return;
	}

	TSharedPtr<SWidget> NewSWidgetRef = NewWidgetPath.GetLastWidget();
	if (!NewSWidgetRef.IsValid())
	{
		//LastFocusedSlotButton = bIsLastFocusedBtnValid ? LastFocusedSlotButton : nullptr;
		//CurrentFocusedSlotButton = bIsCurrentFocusedBtnValid ? CurrentFocusedSlotButton : nullptr;
		return;
	}

	// SButton으로 캐스팅
	TSharedPtr<SButton> NewSButtonWidget = StaticCastSharedPtr<SButton>(NewSWidgetRef);
	if (!NewSButtonWidget.IsValid())
	{
		//LastFocusedSlotButton = bIsLastFocusedBtnValid ? LastFocusedSlotButton : nullptr;
		//CurrentFocusedSlotButton = bIsCurrentFocusedBtnValid ? CurrentFocusedSlotButton : nullptr;
		return;
	}

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
			CurrentFocusedSlotButton = Btn;
			bIsCurrentFocusedBtnValid = true;
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
			CurrentFocusedSlotButton = Btn;
			bIsCurrentFocusedBtnValid = true;
		}
	}

	//LastFocusedSlotButton = bIsLastFocusedBtnValid ? LastFocusedSlotButton : nullptr;
	//CurrentFocusedSlotButton = bIsCurrentFocusedBtnValid ? CurrentFocusedSlotButton : nullptr;
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

void UNAInventoryWidget::SelectInventorySlotWidget() const
{
	if (!CurrentFocusedSlotButton.IsValid()) return;
	
	if (GEngine)
	{
		FString Log = TEXT("SelectInventorySlotButton: ");
		Log += CurrentFocusedSlotButton->GetName();
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Magenta, *Log);
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

void UNAInventoryWidget::RefreshSingleSlotWidget(const FName& SlotID, const UNAItemData* SlotData)
{
	//if (!SlotData) return;
	
	int32 Row, Col, Index = -1;
	if (ParseSlotIDToInvenGrid(SlotID, Row, Col))
	{
		if (!InvenSlotWidgets[Row][Col].IsValid()) return;
		UpdateInvenSlotDrawData(SlotData, InvenSlotWidgets[Row][Col]);
	}
	else if (ParseSlotIDToWeaponIndex(SlotID, Index))
	{
		if (!WeaponSlotWidgets[Index].IsValid()) return;
		UpdateWeaponSlotDrawData(SlotData, WeaponSlotWidgets[Index]);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[UNAInventoryWidget::RefreshSingleSlotWidget]  유효하지 않은 슬롯ID: %s"), *SlotID.ToString());
	}
	
	InvalidateLayoutAndVolatility();
}

void UNAInventoryWidget::RefreshSlotWidgets(const TMap<FName, TWeakObjectPtr<UNAItemData>>& InventoryItems
                                            , const TMap<FName, TWeakObjectPtr<UNAItemData>>& WeaponItems)
{
	if (!ensure(InventoryItems.Num() == MaxInventorySlotCount
		&& WeaponItems.Num() == MaxWeaponSlotCount))
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
		const UNAItemData* FoundItem = nullptr;
		if (FoundItemPtr != nullptr)
		{
			FoundItem = FoundItemPtr->Get();
		}
		
		int32 Index = ExtractSlotNumber(SlotID);
		int32 Row = -1;
		int32 Col = -1;
		const bool bSucceed = ConvertIndexToInvenGrid(Index, Row, Col);
		if (!bSucceed)
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("[RefreshWeaponSlotWidgets]  ConvertIndexToInvenGrid: index -> [Row][Col] 변환실패."));
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
		const TWeakObjectPtr<UNAItemData>* FoundItemPtr = WeaponItems.Find(SlotID);
		const UNAItemData* FoundItem = nullptr;
		if (FoundItemPtr != nullptr)
		{
			FoundItem = FoundItemPtr->Get();
		}
		
		int32 Index = ExtractSlotNumber(SlotID);
		if (!FMath::IsWithinInclusive(Index, 0, MaxWeaponSlotCount))
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

void UNAInventoryWidget::UpdateInvenSlotDrawData(const UNAItemData* ItemData, const FNAInvenSlotWidgets& Inven_SlotWidgets)
{
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
			IconImage->SetBrushResourceObject(Icon);
			IconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		if (NumText)
		{
			NumText->SetText(FText::AsNumber(Quantity));
			NumText->SetVisibility(Quantity > 0 ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
		}
	}
	// 2-B) 아이템이 없는 빈 슬롯이면 아이콘, 텍스트 감추기
	else
	{
		if (IconImage)
		{
			IconImage->SetBrushResourceObject(nullptr);
			IconImage->SetVisibility(ESlateVisibility::Hidden);
		}
		if (NumText)
		{
			NumText->SetText(FText::GetEmpty());
			NumText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UNAInventoryWidget::UpdateWeaponSlotDrawData(const UNAItemData* ItemData, const FNAWeaponSlotWidgets& Weapon_SlotWidgets)
{
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
			IconImage->SetBrushResourceObject(Icon);
			IconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
	// 2-B) 아이템이 없는 빈 슬롯이면 아이콘, 텍스트 감추기
	else
	{
		if (IconImage)
		{
			IconImage->SetBrushResourceObject(nullptr);
			IconImage->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
