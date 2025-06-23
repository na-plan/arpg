// Fill out your copyright notice in the Description page of Project Settings.


#include "NAInGameHUD.h"

#include "ARPG/Public/NAInGameWidget.h"

ANAInGameHUD::ANAInGameHUD(): InGameWidget(nullptr)
{
	if (!InGameWidgetType)
	{
		UE_LOG(LogTemp, Warning, TEXT("Main In-Game widget class is not set, force overriding with default one."));

		static ConstructorHelpers::FClassFinder<UUserWidget> InGameBPClass(
			TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/00_ProjectNA/01_Blueprint/01_Widget/InGame/BP_NAInGameWidget.BP_NAInGameWidget_C'"));
		if (InGameBPClass.Succeeded())
		{
			InGameWidgetType = InGameBPClass.Class;
		}
	}
}

void ANAInGameHUD::OnEscapeDown()
{
	if ( InGameWidget->IsVisible() )
	{
		InGameWidget->SetVisibility(ESlateVisibility::Hidden);
		FInputModeGameOnly InputMode;
		GetOwningPlayerController()->SetInputMode( InputMode );
		GetOwningPlayerController()->SetShowMouseCursor( false );
	}
	else
	{
		InGameWidget->SetVisibility(ESlateVisibility::Visible);
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior( EMouseLockMode::DoNotLock );
		InputMode.SetWidgetToFocus( InGameWidget->TakeWidget() );
		InputMode.SetHideCursorDuringCapture( false );
		GetOwningPlayerController()->SetShowMouseCursor( true );
		GetOwningPlayerController()->SetInputMode( InputMode );
	}
}

void ANAInGameHUD::BeginPlay()
{
	Super::BeginPlay();
	
	if (!InGameWidget)
	{
		InGameWidget = CreateWidget<UNAInGameWidget>(GetWorld(), InGameWidgetType);
		InGameWidget->AddToPlayerScreen();
		InGameWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	GetOwningPlayerController()->InputComponent->BindKey( EKeys::Escape, IE_Pressed, this, &ANAInGameHUD::OnEscapeDown );
}

void ANAInGameHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	InGameWidget->RemoveFromParent();
	Super::EndPlay(EndPlayReason);
}
