// Fill out your copyright notice in the Description page of Project Settings.


#include "CommonWidgets/NAWidgetSubsystem.h"

void UNAWidgetSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	WidgetFolderPath = TEXT("");

	
}

void UNAWidgetSubsystem::OpenWidget(const FString& InWidgetName)
{
	
}

void UNAWidgetSubsystem::CloseWidget(const FString& InWidgetName)
{
}

void UNAWidgetSubsystem::ShowMessageBox(EMessageBoxType Type, const FString& InTitle, const FString& InDesc)
{
	//CachedWidgets[TEXT("MessageBox")]
	
}
