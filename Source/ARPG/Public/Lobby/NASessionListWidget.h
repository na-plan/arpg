// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NAGameInstance.h"
#include "OnlineSessionSettings.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "NASessionListWidget.generated.h"


class UCanvasPanel;
class UNAGameInstance;
class UButton;
class UTextBlock;
class UListView;

UCLASS()
class UNASessionListEntryData : public UObject
{
	GENERATED_BODY()

public:
	UNASessionListEntryData()
	{
	}
		
public:
	bool bPlayerValue = false;
	
	FString SessionName;

	FString TravelAddress;
	
	int32 SessionIndex;
	
	TSharedPtr<FOnlineSessionSearchResult> SearchResult;
};

UCLASS()
class UNASessionListEntry : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override
	{
		UNASessionListEntryData* Result = Cast<UNASessionListEntryData>(ListItemObject);

		if (!Result) return;

		Data = Result;
		
		Text_SessionName->SetText(FText::FromString(Data->SessionName));

		if ( Data->bPlayerValue )
		{
			Button_Join->SetVisibility( ESlateVisibility::Collapsed );
		}
		else
		{
			Button_Join->OnClicked.AddDynamic(this, &ThisClass::OnClick_Join);
		}
	}
	
	virtual void NativeDestruct() override
	{
		Button_Join->OnClicked.Clear();
	};
	
public:
	UFUNCTION()
	void OnClick_Join()
	{
		UNAGameInstance* GameInstance = Cast<UNAGameInstance>(GetGameInstance());
		GameInstance->SetReservedIndex( Data->SessionIndex );
		GameInstance->JoinSession( GetWorld()->GetFirstLocalPlayerFromController(), *Data->SearchResult );
	}
	
public:
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Join;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_SessionName;

protected:
	UPROPERTY()
	UNASessionListEntryData* Data;
};


/************************************************************************************************************************
 *
 *		UNASessionListWidget
 *
 *		@features: 현재 생성된 sessionlist를 보여주고 참여할 수 있게 함
 *		
 */

UCLASS()
class ARPG_API UNASessionListWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	virtual void NativeOnInitialized() override;
	
	virtual void NativeDestruct() override;
	
public:
	UFUNCTION()
	void OnClick_CreateSession();

	UFUNCTION()
	void OnClick_StartGame();

	UFUNCTION()
	void OnClick_Return();

	UFUNCTION()
	void OnClick_Refresh();

	UListView* GetSessionList() const;
	
protected:
	void RefreshSessionList();
	
	void UpdatePlayerStates( [[maybe_unused]] APlayerState* PlayerState );
	
	FCriticalSection ListMutex;
	
protected:
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	UListView* SessionListView;
	
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	UButton* Button_CreateSession;
	
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	UButton* Button_StartGame;

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	UButton* Button_Return;

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	UButton* Button_Refresh;
	
protected:
	TWeakObjectPtr<UNAGameInstance> CachedGameInstance;
	
	TWeakPtr<FOnlineSessionSearch> SessionSearch;
};
