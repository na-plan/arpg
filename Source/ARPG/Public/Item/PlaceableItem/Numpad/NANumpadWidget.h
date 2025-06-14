// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "NANumpadWidget.generated.h"

class ANAPlaceableItemActor;
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

	UFUNCTION()
	void OnClick_CloseButton();
	
private:
	void InitNumber();
	
	void InputNumber();

	void ConfirmNumber();

	void ClearNumber();

	void DetectHoveredButton();
	
	void CloseWidget();
	
public:
	bool IsComplete() const { return bIsComplete; }

	void SetTargetActor(ANAPlaceableItemActor* InActor) { TargetToActivate = InActor; }
	
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

	UPROPERTY(EditAnywhere, meta = (bindwidget))
	UButton* Button_CloseWidget;

	UPROPERTY(EditAnywhere, meta = (bindwidget))
	UImage* InputNum00;

	UPROPERTY(EditAnywhere, meta = (bindwidget))
	UImage* InputNum01;

	UPROPERTY(EditAnywhere, meta = (bindwidget))
	UImage* InputNum02;

	UPROPERTY(EditAnywhere, meta = (bindwidget))
	UImage* InputNum03;
	
	UPROPERTY()
	TArray<UImage*> InputNums;

	UPROPERTY()
	TArray<UButton*> NumButtons;
	
private:
	UPROPERTY()
	int CurrentHoveredButtonIndex = 0;
	
	UPROPERTY()
	int CurrentInputIndex = 0;
	
	UPROPERTY()
	int CurrentInputNumbers[4];

	UPROPERTY()
	int PasswordLength = 4;
	
	UPROPERTY()
	int CorrectPassword = 1024; // temp

	UPROPERTY()
	UTexture2D* CachedNumImage[10];

	UPROPERTY()
	bool bIsComplete = false;

	UPROPERTY()
	APlayerController* CurrentController;

	UPROPERTY()
	ANAPlaceableItemActor* TargetToActivate;
};
