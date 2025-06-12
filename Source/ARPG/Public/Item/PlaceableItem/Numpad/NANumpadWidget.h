// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "NANumpadWidget.generated.h"

class UImage;
class UUniformGridPanel;
/**
 * 
 */
UCLASS()
class ARPG_API UNANumpadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UNANumpadWidget(const FObjectInitializer& ObjectInitializer);

public:
	virtual void NativeConstruct() override;
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UFUNCTION()
	void OnClick_NumButton();

	UFUNCTION()
	void OnClick_ClearButton();

	UFUNCTION()
	void OnClick_ConfirmButton();
	
private:
	void InitNumber();
	
	void InputNumber();

	void ConfirmNumber();

	void ClearNumber();

	void DetectHoveredButton();

public:
	bool IsComplete() const { return bIsComplete; }
	
protected:
	UPROPERTY(EditAnywhere, meta = (bindwidget))
	UButton* Button_Input00;

	UPROPERTY(EditAnywhere, meta = (bindwidget))
	UButton* Button_Input01;

	UPROPERTY(EditAnywhere, meta = (bindwidget))
	UButton* Button_Input02;

	UPROPERTY(EditAnywhere, meta = (bindwidget))
	UButton* Button_Input03;

	UPROPERTY(EditAnywhere, meta = (bindwidget))
	UButton* Button_Input04;

	UPROPERTY(EditAnywhere, meta = (bindwidget))
	UButton* Button_Input05;

	UPROPERTY(EditAnywhere, meta = (bindwidget))
	UButton* Button_Input06;

	UPROPERTY(EditAnywhere, meta = (bindwidget))
	UButton* Button_Input07;

	UPROPERTY(EditAnywhere, meta = (bindwidget))
	UButton* Button_Input08;

	UPROPERTY(EditAnywhere, meta = (bindwidget))
	UButton* Button_Input09;
	
	UPROPERTY(EditAnywhere, meta = (bindwidget))
	UButton* Button_InputClear;

	UPROPERTY(EditAnywhere, meta = (bindwidget))
	UButton* Button_InputConfirm;
	
	UPROPERTY()
	TArray<UImage*> InputNumImages;

	UPROPERTY()
	TArray<UButton*> NumButtons;
	
private:
	UPROPERTY()
	int CurrentHoveredButtonIndex = 0;
	
	UPROPERTY()
	int CurrentInputIndex = 0;
	
	UPROPERTY()
	int CurrentInputNumbers[3];

	UPROPERTY()
	int PasswordLength = 4;
	
	UPROPERTY()
	int CorrectPassword = 1024; // temp

	UPROPERTY()
	UTexture2D* CachedNumImage[9];

	UPROPERTY()
	bool bIsComplete = false;

	UPROPERTY()
	APlayerController* CurrentController;
};
