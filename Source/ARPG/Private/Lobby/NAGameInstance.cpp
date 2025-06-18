// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/NAGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"



UNAGameInstance::UNAGameInstance()
{
}

void UNAGameInstance::Init()
{
	Super::Init();
	
	// LAN 기반 Subsystem, steam이나 다른 플랫폼이면 ini, 플러그인 추가해야함
	IOnlineSubsystem* subsystem =IOnlineSubsystem::Get(FName("NULL"));
	SessionInterface = subsystem->GetSessionInterface();

	if (SessionInterface.IsValid())
	{
		// 콜백함수 바인딩
		SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &ThisClass::OnFindSessionComplete);
		SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &ThisClass::OnCreateSessionComplete);
		SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &ThisClass::OnJoinSessionComplete);
		SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &ThisClass::OnDestroySessionComplete);
	}
}

void UNAGameInstance::FindSessions()
{
	if (!SessionInterface.IsValid()) return;

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 20;
	
	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UNAGameInstance::JoinSession(int32 Index)
{
	if (!SessionInterface.IsValid() || !SessionSearch.IsValid()) return;

	SessionInterface->JoinSession(0, MadeSessionName, SessionSearch->SearchResults[Index]);
}

void UNAGameInstance::JoinSession_Wrapped()
{
	JoinSession(ReservedSessionIndex);
}

void UNAGameInstance::CreateSession(FName SessionName, bool bIsLAN)
{
	if (!SessionInterface.IsValid()) return;

	FOnlineSessionSettings Settings;
	Settings.bIsLANMatch = true;
	Settings.NumPublicConnections = 2;
	Settings.bShouldAdvertise = true;
	MadeSessionName = SessionName;
	
	SessionInterface->CreateSession(0, MadeSessionName, Settings);
}

void UNAGameInstance::DestroySession()
{
}

void UNAGameInstance::OnFindSessionComplete(bool bWasSuccess)
{
	if (bWasSuccess && SessionSearch.IsValid())
	{
		for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Log, TEXT("세션 발견: %s"), *Result.GetSessionIdStr());
		}
	}
}

void UNAGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	FString Str;
	if (SessionInterface->GetResolvedConnectString(SessionName, Str))
	{
		APlayerController* PC = GetFirstLocalPlayerController();
		if (PC)
			PC->ClientTravel(Str, TRAVEL_Absolute);
	}
}

void UNAGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccess)
{
	if (bWasSuccess)
	{
		GetWorld()->ServerTravel("");
	}
}

void UNAGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccess)
{
}
