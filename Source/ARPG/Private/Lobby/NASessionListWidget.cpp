// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/NASessionListWidget.h"

#include "Components/ListView.h"
#include "Lobby/NAGameInstance.h"

void UNASessionListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedGameInstance = Cast<UNAGameInstance>(GetGameInstance());
	
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
	
}
