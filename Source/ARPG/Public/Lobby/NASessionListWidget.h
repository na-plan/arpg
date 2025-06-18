// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "NASessionListWidget.generated.h"


class UTextBlock;
class UListView;

UCLASS()
class UNALobbySessionListEntryData : public UObject
{
	GENERATED_BODY()

public:
	FString SessionName;
};

UCLASS()
class UNASessionListEntry : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override
	{
		UNALobbySessionListEntryData* Result = Cast<UNALobbySessionListEntryData>(ListItemObject);

		if (Result) Data = Result;
	}
	
public:
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Join;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_SessionName;

protected:
	UNALobbySessionListEntryData* Data;
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

protected:
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	UListView* SessionListView;
};
