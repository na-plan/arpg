// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPG/Public/NAPlayerController.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "NAPlayerState.h"

#include "Inventory/NAInventoryComponent.h"
#include "Inventory/Widget/NAInventoryWidget.h"

ANAPlayerController::ANAPlayerController()
{
	InventoryComponent = CreateDefaultSubobject<UNAInventoryComponent>(TEXT("InventoryComponent"));
}

void ANAPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (GetPawn() && !InventoryWidget)
	{
		// TSubclassOf<UNAInventoryWidget> EmotePingMenuWBPClass = StaticLoadClass(
		// 	UNAInventoryWidget::StaticClass(), nullptr, TEXT("위젯 블프 에셋 만들고 경로 변경"));
		// check(EmotePingMenuWBPClass);
		// InventoryWidget = CreateWidget<UNAInventoryWidget>(this, EmotePingMenuWBPClass, TEXT("InventoryWidget"));
	}
}

void ANAPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// if (InteractionComponent)
	// {
	// 	AttachToPawn(InPawn);
	// }
}

void ANAPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);
	
	if (const TScriptInterface<IAbilitySystemInterface> Interface(P); Interface)
	{
		Interface->GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState<ANAPlayerState>(), P);
	}
}

void ANAPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	// if (InteractionComponent)
	// {
	// 	DetachFromPawn();
	// }
}
