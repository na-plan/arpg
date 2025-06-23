// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NAInGameWidget.generated.h"

class UNAHPWidget;
class UButton;
/**
 * 
 */
UCLASS()
class ARPG_API UNAInGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
protected:
	UFUNCTION()
	void OnClick_Continue();

	UFUNCTION()
	void OnClick_Quit();

protected:
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UButton* Button_Continue;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UButton* Button_Quit;
};
