// Fill out your copyright notice in the Description page of Project Settings.


#include "NAInGameWidget.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Lobby/NAGameInstance.h"

void UNAInGameWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Button_Continue->OnClicked.AddDynamic(this, &ThisClass::OnClick_Continue);
	Button_Quit->OnClicked.AddDynamic(this, &ThisClass::OnClick_Quit);
}



void UNAInGameWidget::OnClick_Continue()
{
	SetVisibility(ESlateVisibility::Hidden);
	FInputModeGameOnly InputMode;
	GetOwningPlayer()->SetInputMode( InputMode );
	GetOwningPlayer()->SetShowMouseCursor( false );
}

void UNAInGameWidget::OnClick_Quit()
{
	UGameplayStatics::OpenLevel(GetWorld(),"/Game/00_ProjectNA/02_Level/Level_NALobby", true);
	FInputModeGameOnly InputMode;
	GetOwningPlayer()->SetInputMode( InputMode );
	GetOwningPlayer()->SetShowMouseCursor( false );
	Cast<UNAGameInstance>( GetGameInstance() )->DestroySession();
}

