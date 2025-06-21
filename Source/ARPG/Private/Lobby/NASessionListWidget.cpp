// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/NASessionListWidget.h"

#include "Components/ListView.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Lobby/NAGameInstance.h"

void UNASessionListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedGameInstance = Cast<UNAGameInstance>(GetGameInstance());
	if ( !CachedGameInstance.IsValid() ) return;

	Button_CreateSession->OnClicked.AddDynamic(this,&ThisClass::OnClick_CreateSession);
	Button_StartGame->OnClicked.AddDynamic(this, &ThisClass::OnClick_StartGame);
	Button_Return->OnClicked.AddDynamic(this, &ThisClass::OnClick_Return);
	Button_Refresh->OnClicked.AddDynamic(this, &ThisClass::OnClick_Refresh);
	CachedGameInstance->OnSessionFound.AddLambda([&](){ RefreshSessionList();});
}

void UNASessionListWidget::OnClick_CreateSession()
{
	//UGameplayStatics::OpenLevel(GetWorld(),"/Game/00_ProjectNA/02_Level/Level_NALobby?listen");
	//SetVisibility(ESlateVisibility::Visible);
	CreateSession();

	Button_StartGame->SetVisibility( ESlateVisibility::Visible );
}

void UNASessionListWidget::OnClick_StartGame()
{
	CachedGameInstance->StartSession_Wrapped();
}

void UNASessionListWidget::OnClick_Return()
{
	SetVisibility( ESlateVisibility::Hidden );
}

void UNASessionListWidget::OnClick_Refresh()
{
	CachedGameInstance->FindSessions();
}

void UNASessionListWidget::RefreshSessionList()
{
	if ( ListMutex.TryLock() )
	{
		SessionSearch = CachedGameInstance->GetSessionSearch();

		auto Results = SessionSearch.Pin()->SearchResults;

		if (!Results.IsEmpty())
		{
			for (auto& It :  Results)
			{
				UNASessionListEntryData* Data = NewObject<UNASessionListEntryData>();
				Data->SearchResult = MakeShared<FOnlineSessionSearchResult>(It);

				SessionListView->AddItem( Data );
			}
		}
	
		if ( !CachedGameInstance->IsHosting() )
		{
			Button_StartGame->SetVisibility( ESlateVisibility::Collapsed );
		}

		ListMutex.Unlock();
	}
}

void UNASessionListWidget::CreateSession()
{
	auto SessionName = FString::FromInt(GetOwningPlayerState()->GetPlayerId());
	CachedGameInstance->CreateSession(FName(SessionName), true);
	Button_StartGame->SetVisibility( ESlateVisibility::Visible );
}
