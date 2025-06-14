﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PlaceableItem/Numpad//NANumpadWidget.h"

#include "Components/Image.h"
#include "Item/ItemActor/NAPlaceableItemActor.h"
#include "Misc/NAWorldEventHandler.h"

UNANumpadWidget::UNANumpadWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UNANumpadWidget::NativeConstruct()
{
	Super::NativeConstruct();

	for (int i = 0; i < 9; i++)
	{
		FString FilePath = FString::Printf(TEXT("/Script/Engine.Texture2D'/Game/00_ProjectNA/05_Resource/06_Widget/Numpad/%i_2.%i_2'"),i, i);
		CachedNumImage[i] = LoadObject<UTexture2D>(this, *FilePath);
	}
	
	NumButtons.Reserve(10);
	NumButtons.Add(Button_Input00);
	NumButtons.Add(Button_Input01);
	NumButtons.Add(Button_Input02);
	NumButtons.Add(Button_Input03);
	NumButtons.Add(Button_Input04);
	NumButtons.Add(Button_Input05);
	NumButtons.Add(Button_Input06);
	NumButtons.Add(Button_Input07);
	NumButtons.Add(Button_Input08);
	NumButtons.Add(Button_Input09);

	InputNums.Reserve(4);
	InputNums.Add(InputNum00);
	InputNums.Add(InputNum01);
	InputNums.Add(InputNum02);
	InputNums.Add(InputNum03);
	
	if (!NumButtons.IsEmpty())
	{
		for (UButton* Button : NumButtons)
			Button->OnClicked.AddDynamic(this, &ThisClass::OnClick_NumButton);
	}

	if (!InputNums.IsEmpty())
	{
		for (UImage* Image : InputNums)
			Image->SetBrushFromTexture(CachedNumImage[0]);
	}
	
	if (Button_InputClear)
		Button_InputClear->OnClicked.AddDynamic(this, &ThisClass::OnClick_ClearButton);
	
	if (Button_InputConfirm)
		Button_InputConfirm->OnClicked.AddDynamic(this, &ThisClass::OnClick_ConfirmButton);

	if (Button_CloseWidget)
		Button_CloseWidget->OnClicked.AddDynamic(this, &ThisClass::OnClick_CloseButton);
	
	CurrentController = GetWorld()->GetFirstPlayerController();
}

void UNANumpadWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsComplete) return;
	
	bool bIsVisible = GetVisibility() == ESlateVisibility::Visible;
	if (!bIsVisible)
	{
		CurrentController->SetInputMode(FInputModeGameAndUI());
		return;
	}

	//CurrentController->SetInputMode(FInputModeUIOnly());
	CurrentController->bShowMouseCursor = bIsVisible;
	
	DetectHoveredButton();
}

void UNANumpadWidget::InitNumber()
{
	if (bIsComplete) return;
	
	for (int& Num :CurrentInputNumbers)
	{
		Num = 0;
	}
	
	for (UImage* InputNumImage : InputNums)
	{
		InputNumImage->SetBrushFromTexture(CachedNumImage[0]);
	}

	CurrentInputIndex = 0;
}

void UNANumpadWidget::OnClick_NumButton()
{
	InputNumber();
}

void UNANumpadWidget::OnClick_ClearButton()
{
	ClearNumber();
}

void UNANumpadWidget::OnClick_ConfirmButton()
{
	ConfirmNumber();
}

void UNANumpadWidget::OnClick_CloseButton()
{
	CloseWidget();
}

void UNANumpadWidget::InputNumber()
{
	if (PasswordLength <= CurrentInputIndex) return;
	
	CurrentInputNumbers[CurrentInputIndex] = CurrentHoveredButtonIndex;
	
	InputNums[CurrentInputIndex]->SetBrushFromTexture(CachedNumImage[CurrentHoveredButtonIndex]);

	++CurrentInputIndex;
}

void UNANumpadWidget::ClearNumber()
{
	InitNumber();
}

void UNANumpadWidget::DetectHoveredButton()
{
	int Count = 9;

	for (int i = 0; i < Count; i++)
	{
		if (NumButtons[i]->IsHovered())
		{
			FString Number = NumButtons[i]->GetParent()->GetName();
			CurrentHoveredButtonIndex = FCString::Atoi(*Number);
		}
	}
}

void UNANumpadWidget::CloseWidget()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UNANumpadWidget::ConfirmNumber()
{
	if (bIsComplete) return;
	
	int Password = CorrectPassword;
	
	for (int i = PasswordLength - 1; i > 0; --i)
	{
		if (CurrentInputNumbers[i] == Password % 10)
		{
			Password /= 10;
		}
		else
		{
			return;
		}
	}

	bIsComplete = true;
	SetVisibility(ESlateVisibility::Hidden);
	CurrentController->SetInputMode(FInputModeGameAndUI());
	CurrentController->bShowMouseCursor = true;

	//UNAWorldEventHandler::GetInstance()->TriggerEvent(TEXT("Test"));
}
