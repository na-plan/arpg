// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/NASessionListWidget.h"

#include "Components/ListView.h"
#include "GameFramework/PlayerState.h"
#include "Lobby/NAGameInstance.h"

void UNASessionListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedGameInstance = Cast<UNAGameInstance>(GetGameInstance());

	if (!CachedGameInstance) return;

	Button_CreateSession->OnClicked.AddDynamic(this,&ThisClass::OnClick_CreateSession);
	Button_StartGame->OnClicked.AddDynamic(this, &ThisClass::OnClick_StartGame);

	RefreshSessionList();
}

void UNASessionListWidget::OnClick_CreateSession()
{
	CreateSession();
}

void UNASessionListWidget::OnClick_StartGame()
{
	//GetWorld()->ServerTravel();
}

void UNASessionListWidget::RefreshSessionList()
{
	CachedGameInstance->FindSessions();
	SessionSearch = CachedGameInstance->GetSessionSearch();

	auto Results = SessionSearch->SearchResults;

	if (Results.IsEmpty()) return;
	
	for (auto& It :  Results)
	{
		UNASessionListEntryData Data;
		Data.SessionName = It.Session.OwningUserName;

		SessionListView->AddItem(Cast<UObject>(&Data));
	}

	if (!GetWorld()->IsNetMode(NM_ListenServer))
		Button_StartGame->SetVisibility(ESlateVisibility::Hidden);
}

void UNASessionListWidget::CreateSession()
{
	auto SessionName = GetOwningPlayerState()->SessionName;
	CachedGameInstance->CreateSession(FName(SessionName), true);
	RefreshSessionList();
}
