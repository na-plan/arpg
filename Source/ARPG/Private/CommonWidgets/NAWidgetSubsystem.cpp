// Fill out your copyright notice in the Description page of Project Settings.


#include "CommonWidgets/NAWidgetSubsystem.h"

#include "AudioDevice.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"
#include "Engine/Blueprint.h"
#include "Kismet/GameplayStatics.h"

void UNAWidgetSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	WidgetFolderPath = TEXT("/Game/00_ProjectNA/01_Blueprint/01_Widget/Common");

	// FAssetRegistryModule& module = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	//
	// FARFilter filter;
	// filter.PackagePaths.Add(*WidgetFolderPath);
	// filter.ClassNames.Add(UUserWidget::StaticClass()->GetFName());
	// //filter.ClassPaths.Add(FTopLevelAssetPath(TEXT("/Script/UMG.UserWidget")));
	// filter.bRecursivePaths = true;
	// filter.bRecursiveClasses = true;
	//
	//
	// TArray<FAssetData> assetList;
	// module.Get().GetAssets(filter,assetList);
	//
	// for (const FAssetData& asset : assetList)
	// {
	// 	UBlueprint* bp = Cast<UBlueprint>(asset.GetAsset());
	// 	if (!bp) continue;
	//
	// 	UClass* generatedClass = bp->GeneratedClass;
	// 	
	// 	if (generatedClass && generatedClass->IsChildOf(UUserWidget::StaticClass()))
	// 	{
	// 		TSubclassOf<UUserWidget> widgetClass = Cast<UClass>(generatedClass);
	//
	// 		if (widgetClass)
	// 			CachedWidgets.Emplace(widgetClass->GetName(), widgetClass);
	// 	}
	// }

	// defaultengine.ini에 등록된 primary asset으로 등록된 widget들을 불러옴
	// projectsetting -> assetmanager 에서도 수정가능함
	UAssetManager& manager = UAssetManager::Get();
	TArray<FPrimaryAssetId> widgetAssets;
	manager.GetPrimaryAssetIdList(FPrimaryAssetType("UserWidget"), widgetAssets);
	
	for (const FPrimaryAssetId& assetId : widgetAssets)
	{
		FSoftObjectPath widgetClassPath = manager.GetPrimaryAssetPath(assetId);
		FString assetName = widgetClassPath.GetAssetName();
		FString keyName = assetName.Replace(TEXT("BP_"),TEXT("U")).Replace(TEXT("_C"),TEXT(""));
		
		UClass* rawClass = StaticLoadClass(
			UUserWidget::StaticClass(),
			nullptr,
			*widgetClassPath.ToString()
			);

		TSubclassOf<UUserWidget> widgetClass = Cast<UClass>(rawClass);
		CachedWidgets.Emplace(keyName, widgetClass);
	}
}

template<class WidgetType>
WidgetType* UNAWidgetSubsystem::OpenWidget()
{
	// 클래스이름으로 불러온다
	// ex) BP_NATempWidget -> UNATempWidget
	
	FString typeName;
	
	if (TIsDerivedFrom<WidgetType, UObject>::IsDerived)
		typeName = WidgetType::StaticClass()->GetName();

	if (ActiveWidgets.Contains(typeName)) return Cast<WidgetType>(ActiveWidgets[typeName]);

	TSubclassOf<UUserWidget> widgetClass = CachedWidgets[typeName];
	UUserWidget* widget = CreateWidget<UUserWidget>(GetWorld(), widgetClass);
	widget->AddToViewport();
	ActiveWidgets.Emplace(typeName, widget);
	
	return Cast<WidgetType>(CachedWidgets[typeName]);
}

void UNAWidgetSubsystem::CloseWidget(const FString& InWidgetName)
{
	// 클래스 이름으로 삭제한다. (open함수 참조)
	
	if (!ActiveWidgets.Find(InWidgetName)) return;
	
	ActiveWidgets[InWidgetName]->RemoveFromRoot();
	ActiveWidgets.Remove(InWidgetName);
}

void UNAWidgetSubsystem::ShowMessageBox(const UWorld* WorldContext, EMessageBoxType Type, const FString& InTitle, const FString& InDesc, TFunction<void()> InFunction)
{
	// 서브시스템 사용예시
	// UNAWidgetSubsystem* instance = WorldContext->GetGameInstance()->GetSubsystem<UNAWidgetSubsystem>();
	// UNAMessageBox* messageBox = instance->OpenWidget<UNAMessageBox>();
	//
	// messageBox->ShowMessage(Type, InTitle, InDesc, InFunction);
}
