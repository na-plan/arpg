// Fill out your copyright notice in the Description page of Project Settings.


#include "CommonWidgets/NAMessageBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UNAMessageBox::NativeConstruct()
{
	Super::NativeConstruct();

	Text_Title->SetText(FText::FromString(TEXT("")));
	Text_Desc->SetText(FText::FromString(TEXT("")));
	
	Button_OK->SetVisibility(ESlateVisibility::Hidden);
	Button_Yes->SetVisibility(ESlateVisibility::Hidden);
	Button_No->SetVisibility(ESlateVisibility::Hidden);

	Button_OK->OnClicked.AddDynamic(this, &ThisClass::OnClick_OK);
	Button_Yes->OnClicked.AddDynamic(this, &ThisClass::OnClick_Yes);
	Button_No->OnClicked.AddDynamic(this, &ThisClass::OnClick_No);
}

void UNAMessageBox::OnClick_OK()
{
	AwaitClick();
}

void UNAMessageBox::OnClick_Yes()
{
	AwaitClick();
}

void UNAMessageBox::OnClick_No()
{
	RemoveFromParent();
}

void UNAMessageBox::ShowMessage(EMessageBoxType InType, const FString& InTitle, const FString& InDesc,
	TFunction<void()> InCallback)
{
	switch (InType)
	{
	case EMessageBoxType::YesNo:
		{
			Button_OK->SetVisibility(ESlateVisibility::Hidden);
			Button_Yes->SetVisibility(ESlateVisibility::Visible);
			Button_No->SetVisibility(ESlateVisibility::Visible);
		}
		break;
	default:
		{
			Button_OK->SetVisibility(ESlateVisibility::Visible);
		}
		break;
	}

	Text_Title->SetText(FText::FromString(InTitle));
	Text_Desc->SetText(FText::FromString(InDesc));
	Callback = MakeShareable(&InCallback);
}


void UNAMessageBox::AwaitClick()
{
	if (Callback.IsValid() && *Callback)
		(*Callback)();
	
	RemoveFromParent();
}
