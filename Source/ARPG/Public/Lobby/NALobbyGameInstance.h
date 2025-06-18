// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Engine/GameInstance.h"
#include "NALobbyGameInstance.generated.h"

/************************************************************************************************
 *
 *		UNALobbyGameInstance
 *
 *		@feature: Session 관련 기능들을 관리
 *		
 */

DECLARE_MULTICAST_DELEGATE(FOnSessionSearchComplete);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionFound, const FOnlineSessionSearchResult&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionJoinComplete, EOnJoinSessionCompleteResult::Type);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionCreateComplete, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionDestroyComplete, bool);

UCLASS()
class ARPG_API UNALobbyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UNALobbyGameInstance();

public:
	UFUNCTION()
	void FindSessions();
	
	UFUNCTION()
	void JoinSession(int32 Index);

	UFUNCTION()
	void CreateSession(FName SessionName, bool bIsLAN);

	UFUNCTION()
	void DestroySession();

private:
	void OnFindSessionComplete(bool bWasSuccess);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccess);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccess);
	
private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	//TArray<FOnlineSessionSearchResult*> SessionSearchResults;

	UPROPERTY()
	FName MadeSessionName = FName("TestSession");


	// public:
	// 	FOnSessionSearchComplete OnSessionSearchCompleteDelegate;
	// 	FOnSessionFound OnSessionFoundCompleteDelegate;
	// 	FOnSessionJoinComplete OnSessionJoinCompleteDelegate;
	// 	FOnSessionCreateComplete OnSessionCreateCompleteDelgate;
	// 	FOnSessionDestroyComplete OnSessionDestroyCompleteDelegate;
};
