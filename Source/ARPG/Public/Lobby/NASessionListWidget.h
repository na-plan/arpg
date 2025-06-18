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


class UNAGameInstance;
class UButton;
class UTextBlock;
class UListView;

UCLASS()
class UNASessionListEntryData : public UObject
{
	GENERATED_BODY()

public:
	FString SessionName;
	int32 SessionIndex;
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
		
		UNAGameInstance* GameInstance = Cast<UNAGameInstance>(GetGameInstance());
		GameInstance->SetReservedIndex(Data->SessionIndex);
		Button_Join->OnClicked.AddDynamic(GameInstance,&UNAGameInstance::JoinSession_Wrapped);
	}

	virtual void NativeDestruct() override
	{
		Button_Join->OnClicked.Clear();
	};
	
public:
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Join;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_SessionName;

protected:
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

protected:
	void RefreshSessionList();
	
protected:
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	UListView* SessionListView;

protected:
	UNAGameInstance* CachedGameInstance;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
};
