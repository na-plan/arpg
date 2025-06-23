// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/NALobbyWidget.h"

#include "CommonWidgets/NAWidgetSubsystem.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

UNALobbyWidget::UNALobbyWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UNALobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Button_SingleMode->OnClicked.AddDynamic(this, &ThisClass::OnClick_SingleMode);
	Button_CoopMode->OnClicked.AddDynamic(this, &ThisClass::OnClick_CoopMode);
	SessionListWidget->SetVisibility(ESlateVisibility::Hidden);	

	// test
	// UNAWidgetSubsystem::ShowMessageBox(GetWorld(),
	// 	EMessageBoxType::Ok,
	// 	TEXT("테스트"),
	// 	TEXT("테스트"),
	// 	[&]()
	// 	{
	// 		OnClick_SingleMode();
	// 	});
}

void UNALobbyWidget::OnClick_SingleMode()
{
	UGameplayStatics::OpenLevel(GetWorld(),"/Game/00_ProjectNA/02_Level/Level_NATestLevel");
}

void UNALobbyWidget::OnClick_CoopMode()
{
	SessionListWidget->SetVisibility(ESlateVisibility::Visible);
}

// void UNALobbyWidget::AddSessionEntry(const FString& InSessionName)
// {
// 	if (!SessionEntryClass || !SessionScrollBox) return;
//
// 	UUserWidget* Entry = CreateWidget<UUserWidget>(GetWorld(), SessionEntryClass);
//
// 	if (!Entry) return;
//
// 	UTextBlock* SessionText = Cast<UTextBlock>(Entry->GetWidgetFromName("SessionName"));
//
// 	if (!SessionText) return;
//
// 	SessionText->SetText(FText::FromString(InSessionName));
//
// 	SessionScrollBox->AddChild(Entry);
// }
