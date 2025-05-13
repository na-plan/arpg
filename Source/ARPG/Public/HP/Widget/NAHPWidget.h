// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NAHPWidget.generated.h"

class UProgressBar;
/**
 * 
 */
UCLASS()
class ARPG_API UNAHPWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	
private:

	// HPBar를 업데이트 하는 함수
	UFUNCTION()
	float GetHealthPercentage();

	// 전체 체력 비례 남은 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidget))
	UProgressBar* HPBar;
};
