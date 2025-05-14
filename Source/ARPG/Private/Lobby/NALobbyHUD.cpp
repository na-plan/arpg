// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/NALobbyHUD.h"

#include "Blueprint/UserWidget.h"
#include "Lobby/NALobbyWidget.h"

ANALobbyHUD::ANALobbyHUD()
{
	if (LobbyWidget == nullptr)
	{
		static ConstructorHelpers::FClassFinder<UUserWidget> FoundWidget(TEXT(
			"/Script/UMGEditor.WidgetBlueprint'/Game/00_ProjectNA/02_Widget/00_Lobby/BP_NALobbyWidget.BP_NALobbyWidget_C'"));
		if (FoundWidget.Succeeded())
			LobbyWidgetBP = FoundWidget.Class;
	}
}

void ANALobbyHUD::BeginPlay()
{
	Super::BeginPlay();

	LobbyWidget = CreateWidget<UNALobbyWidget>(GetWorld(), LobbyWidgetBP,TEXT("LobbyWidget"));
	LobbyWidget->AddToViewport();
	GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(true);
}

void ANALobbyHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(false);
}
