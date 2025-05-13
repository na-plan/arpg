// Fill out your copyright notice in the Description page of Project Settings.


#include "NAInGameHUD.h"

#include "ARPG/Public/NAInGameWidget.h"

ANAInGameHUD::ANAInGameHUD(): InGameWidget(nullptr)
{
	if (!InGameWidgetType)
	{
		UE_LOG(LogTemp, Warning, TEXT("Main In-Game widget class is not set, force overriding with default one."));

		static ConstructorHelpers::FClassFinder<UUserWidget> InGameBPClass(
			TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/BP_NAInGameWidget.BP_NAInGameWidget_C'"));
		if (InGameBPClass.Succeeded())
		{
			InGameWidgetType = InGameBPClass.Class;
		}
	}
}

void ANAInGameHUD::BeginPlay()
{
	Super::BeginPlay();
	
	if (!InGameWidget)
	{
		InGameWidget = CreateWidget<UNAInGameWidget>(GetWorld(), InGameWidgetType);
		InGameWidget->AddToPlayerScreen();
	}
}

void ANAInGameHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	InGameWidget->RemoveFromParent();
	Super::EndPlay(EndPlayReason);
}
