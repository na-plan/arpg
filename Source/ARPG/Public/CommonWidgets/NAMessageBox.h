// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"
#include "Blueprint/UserWidget.h"
#include "NAMessageBox.generated.h"

/******************************************************************************************************************************
 *
 *		UNAMessageBox
 *
 *		@features: 확인창을 띄우고 확인시 콜백함수 호출
 *		
 */

class UTextBlock;
class UButton;

UENUM()
enum class EMessageBoxType : uint8
{
	Ok,			// 확인 버튼
	YesNo,		// 예, 아니오 버튼
	Max
};


UCLASS()
class ARPG_API UNAMessageBox : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
public:
	UFUNCTION()
	void OnClick_OK();

	UFUNCTION()
	void OnClick_Yes();
	
	UFUNCTION()
	void OnClick_No();

public:
	void ShowMessage(EMessageBoxType InType, const FString& InTitle, const FString& InDesc, TFunction<void()> InCallback);

	UFUNCTION()
	void AwaitClick();
	
protected:
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	UTextBlock* Text_Title;

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	UTextBlock* Text_Desc;

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	UButton* Button_OK;

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	UButton* Button_Yes;

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true))
	UButton* Button_No;

protected:
	TSharedPtr<TFunction<void()>> Callback;
};
