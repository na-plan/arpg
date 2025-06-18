// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NALobbyGameMode.h"
#include "NASessionListWidget.h"
#include "NALobbyWidget.generated.h"




/************************************************************************************************************************
 *
 *		UNALobbyWidget
 *
 *		@featrue: 게임 진입을 관리
 */
UCLASS()
class ARPG_API UNALobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UNALobbyWidget(const FObjectInitializer& ObjectInitializer);

public:
	virtual void NativeConstruct() override;
	
public:
	UFUNCTION()
	void OnClick_SingleMode();

	UFUNCTION()
	void OnClick_CoopMode();
	
// public:
// 	UFUNCTION()
// 	void AddSessionEntry(const FString& InSessionName);
	
protected:
	UPROPERTY(EditAnywhere)
	ANALobbyGameMode* CachedGameMode;

	UPROPERTY(EditAnywhere, meta = (BindWidget, AllowPrivateAccess = true))
	UNASessionListWidget* SessionListWidget;
	
protected:
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	UButton* Button_SingleMode;

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	UButton* Button_CoopMode;

	// UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	// class UScrollBox* SessionScrollBox;
	//
	// UPROPERTY(EditAnywhere)
	// TSubclassOf<UUserWidget> SessionEntryClass;
};
