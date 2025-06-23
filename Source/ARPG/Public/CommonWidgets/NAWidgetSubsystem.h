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
	template<class WidgetType>
	WidgetType* OpenWidget();

	void CloseWidget(const FString& InWidgetName);
	
public:
	static void ShowMessageBox(const UWorld* WorldContext, EMessageBoxType Type, const FString& InTitle, const FString& InDesc, TFunction<void()> InFunction);
	
protected:
	UPROPERTY()
	FString WidgetFolderPath;

	UPROPERTY()
	TMap<FString, TSubclassOf<UUserWidget>> CachedWidgets;

	UPROPERTY()
	TMap<FString, UUserWidget*> ActiveWidgets;
};
