// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/NAGameInstance.h"

#include "NAPlayerState.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Online/OnlineSessionNames.h"


UNAGameInstance::UNAGameInstance()
{
}

void UNAGameInstance::Init()
{
	Super::Init();
	
	// LAN 기반 Subsystem, steam이나 다른 플랫폼이면 ini, 플러그인 추가해야함
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem( GetWorld(), FName("NULL"));
	SessionInterface = Subsystem->GetSessionInterface();
	
	if ( const TSharedPtr<IOnlineSession> Session = SessionInterface.Pin() )
	{
		// 콜백함수 바인딩
		Session->OnFindSessionsCompleteDelegates.AddUObject(this, &ThisClass::OnFindSessionComplete);
		Session->OnCreateSessionCompleteDelegates.AddUObject(this, &ThisClass::OnCreateSessionComplete);
		Session->OnJoinSessionCompleteDelegates.AddUObject(this, &ThisClass::OnJoinSessionComplete);
		Session->OnDestroySessionCompleteDelegates.AddUObject(this, &ThisClass::OnDestroySessionComplete);
		Session->OnStartSessionCompleteDelegates.AddUObject(this, &ThisClass::OnStartSessionComplete);
	}
}

void UNAGameInstance::FindSessions()
{
	if ( const TSharedPtr<IOnlineSession> Session = SessionInterface.Pin() )
	{
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		SessionSearch->bIsLanQuery = true;
		SessionSearch->MaxSearchResults = 20;
		SessionSearch->TimeoutInSeconds = 1.f;
		SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

		Session->FindSessions( 0, SessionSearch.ToSharedRef() );
	}
}

void UNAGameInstance::JoinSessionByIndex(int32 Index)
{
	if ( !SessionSearch.IsValid() || !SessionSearch->SearchResults.IsValidIndex( Index ) ) return;

	if ( const TSharedPtr<IOnlineSession> Session = SessionInterface.Pin() )
	{
		bHasJoined = Session->JoinSession(0, MadeSessionName, SessionSearch->SearchResults[ Index ]);
	}
}

void UNAGameInstance::JoinSession_Wrapped()
{
	JoinSessionByIndex(ReservedSessionIndex);
}

bool UNAGameInstance::JoinSession( ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult )
{
	if ( const TSharedPtr<IOnlineSession> Session = SessionInterface.Pin() )
	{
		bHasJoined = Session->JoinSession( LocalPlayer->GetLocalPlayerIndex(), MadeSessionName, SearchResult );
		return bHasJoined;
	}

	bHasJoined = false;
	return false;
}

void UNAGameInstance::CreateSession(FName SessionName, bool bIsLAN = true)
{
	if ( const TSharedPtr<IOnlineSession> Session = SessionInterface.Pin() )
	{
		FOnlineSessionSettings Settings;
		Settings.bIsLANMatch = bIsLAN;
		Settings.NumPublicConnections = 2;
		Settings.bShouldAdvertise = true;
		Settings.bUsesPresence = true;
		Settings.bAllowJoinInProgress = true;
		Settings.bAllowJoinViaPresence = true;
		MadeSessionName = SessionName;
		
		Session->CreateSession(0, MadeSessionName, Settings);
		bIsHosting = true;
	}
}

void UNAGameInstance::DestroySession()
{
	if ( const TSharedPtr<IOnlineSession>& Session = SessionInterface.Pin() )
	{
		Session->DestroySession( MadeSessionName );
		MadeSessionName = NAME_None;

		if ( HasJoined() )
		{
			// todo: 로비 레벨 하드코딩!
			GetFirstLocalPlayerController()->ClientTravel( TEXT("/Game/00_ProjectNA/02_Level/Level_NALobby"), TRAVEL_Absolute, true );
			bHasJoined = false;
		}

		bIsHosting = false;
	}
}

void UNAGameInstance::StartSession()
{
	if ( MadeSessionName == NAME_None )
	{
		return;
	}
	
	if ( const TSharedPtr<IOnlineSession>& Session = SessionInterface.Pin() )
	{
		Session->StartSession( MadeSessionName );
	}
}

void UNAGameInstance::StartSession_Wrapped()
{
	StartSession();
}

bool UNAGameInstance::IsHosting() const
{
	return bIsHosting;
}

bool UNAGameInstance::HasJoined() const
{
	return bHasJoined;
}

TWeakPtr<IOnlineSession> UNAGameInstance::GetCurrentSession() const
{
	return SessionInterface;
}

void UNAGameInstance::OnFindSessionComplete(bool bWasSuccess)
{
	if (bWasSuccess && SessionSearch.IsValid())
	{
		for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Log, TEXT("세션 발견: %s"), *Result.GetSessionIdStr());

			OnSessionFound.Broadcast();
		}
	}
}

void UNAGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	FString Str;
	if ( const TSharedPtr<IOnlineSession> Session = SessionInterface.Pin() )
	{
		if (Session->GetResolvedConnectString(SessionName, Str))
		{
			APlayerController* PC = GetFirstLocalPlayerController();
			if (PC)
			{
				auto PlayerState = PC->GetPlayerState<ANAPlayerState>();
				PlayerState->bInSession = true;
				PC->ClientTravel(Str, TRAVEL_Absolute);
			}
		}
	}
}

void UNAGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccess)
{
	if (bWasSuccess)
	{
		//GetWorld()->ServerTravel("");
		UGameplayStatics::OpenLevel(GetWorld(), "/Game/00_ProjectNA/02_Level/Level_NALobby",true,"listen");
		//UGameplayStatics::OpenLevel(GetWorld(),"/Game/00_ProjectNA/02_Level/Level_NALobby?listen");
	}
}

void UNAGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccess)
{
}

void UNAGameInstance::OnStartSessionComplete(FName SessionName, bool bWasSuccess)
{
	if (bWasSuccess)
	{
		GetWorld()->ServerTravel("/Game/00_ProjectNA/02_Level/Level_NATestLevel?listen");		
	}
}
