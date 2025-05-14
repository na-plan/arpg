// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NALobbyWidget.generated.h"

/************************************************************************************************
 *
 *		UNALobbyWidget
 *		@feature: 로비레벨에서 쓰일 위젯들의 모음
 *
 *		
 ************************************************************************************************/

UCLASS()
class ARPG_API UNALobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UNALobbyWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;
	\
private:
	void InitWidgets();
	
	UFUNCTION()
	void OnClick_SinglePlay();

	UFUNCTION()
	void OnClick_CoopPlay();

	UFUNCTION()
	void OnClick_TestPlay();
	
private:
	UPROPERTY(meta = (AllowPrivateAccess = "true", BindWidget))
	class UButton* Button_SinglePlay;

	UPROPERTY(meta = (AllowPrivateAccess = "true", BindWidget))
	class UButton* Button_CoopPlay;

	UPROPERTY(meta = (AllowPrivateAccess = "true", BindWidget))
	class UButton* Button_TestPlay;
};
