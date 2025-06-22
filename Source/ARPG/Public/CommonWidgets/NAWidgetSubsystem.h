// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NAMessageBox.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NAWidgetSubsystem.generated.h"

/************************************************************************************************************************************************
 *
 *		UNAWidgetSubsystem
 *		
 *		@feature: 게임 Screen에 띄우는 widget들을 관리한다.
 *
 * 
 */

UCLASS()
class ARPG_API UNAWidgetSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:
	UFUNCTION()
	void OpenWidget(const FString& InWidgetName);

	UFUNCTION()
	void CloseWidget(const FString& InWidgetName);
	
public:
	UFUNCTION()
	void ShowMessageBox(EMessageBoxType Type, const FString& InTitle, const FString& InDesc);
	
protected:
	UPROPERTY()
	FString WidgetFolderPath;

	UPROPERTY()
	TMap<FString, UUserWidget*> CachedWidgets;
};
