// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PlaceableItem/NANumpadWidget.h"

#include "Components/Image.h"

UNANumpadWidget::UNANumpadWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	for (int i = 0; i < 9; i++)
	{
		static ConstructorHelpers::FObjectFinder<UTexture2D> FoundImage(TEXT("/Script/Engine.Texture2D'/Game/00_ProjectNA/05_Resource/06_Widget/Numpad/%i_2.%i_2'"),i);

		if (FoundImage.Succeeded())
			CachedNumImage[i] = FoundImage.Object;
	}
}

void UNANumpadWidget::NativeConstruct()
{
	Super::NativeConstruct();

	NumButtons.Reserve(9);
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
		
	if (!NumButtons.IsEmpty())
	{
		for (UButton* Button : NumButtons)
			Button->OnClicked.AddDynamic(this, &ThisClass::OnClick_NumButton);
	}
	
	if (Button_InputClear)
		Button_InputClear->OnClicked.AddDynamic(this, &ThisClass::OnClick_ClearButton);
	
	if (Button_InputConfirm)
		Button_InputConfirm->OnClicked.AddDynamic(this, &ThisClass::OnClick_ConfirmButton);
}

void UNANumpadWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsComplete) return;

	DetectHoveredButton();
}

void UNANumpadWidget::InitNumber()
{
	for (int& Num :CurrentInputNumbers)
	{
		Num = 0;
	}
	
	for (UImage* InputNumImage : InputNumImages)
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

void UNANumpadWidget::InputNumber()
{
	if (PasswordLength >= CurrentInputIndex) return;
	
	CurrentInputNumbers[CurrentInputIndex] = CurrentHoveredButtonIndex;\
	
	InputNumImages[CurrentInputIndex]->SetBrushFromTexture(CachedNumImage[CurrentHoveredButtonIndex]);
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

void UNANumpadWidget::ConfirmNumber()
{
	int Password = CorrectPassword;
	
	for (int i = 0; i < PasswordLength; ++i)
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
}
