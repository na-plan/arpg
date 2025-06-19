// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/NALobbyHUD.h"
#include "CineCameraActor.h"
#include "Blueprint/UserWidget.h"	
#include "Item/ItemActor/NAItemActor.h"
#include "Components/DirectionalLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Lobby/NALobbyWidget.h"

ANALobbyHUD::ANALobbyHUD()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> widget(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/00_ProjectNA/01_Blueprint/01_Widget/Lobby/BP_NALobby.BP_NALobby_C'"));
	if (widget.Succeeded())
		LobbyWidgetClass = widget.Class;

	static ConstructorHelpers::FClassFinder<UUserWidget> widget2(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/00_ProjectNA/01_Blueprint/01_Widget/Lobby/BP_NASessionListWidget.BP_NASessionListWidget_C'"));
	if (widget2.Succeeded())
		SessionListWidgetClass = widget2.Class;
}

void ANALobbyHUD::BeginPlay()
{
	Super::BeginPlay();

	
	ACineCameraActor* Cam = nullptr;
	UDirectionalLightComponent* LightComp = nullptr;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->GetName().Contains(TEXT("CineCameraActor")))
		{
			Cam = Cast<ACineCameraActor>(*It);
		}

		if (It->GetName().Contains(TEXT("DirectionalLight")))
		{
			LightComp = It->GetComponentByClass<UDirectionalLightComponent>();
		}
	}
	
	APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	Controller->bShowMouseCursor = true;
	Controller->SetViewTarget(Cam);

	UUserWidget* instance = CreateWidget<UUserWidget>(GetWorld(), LobbyWidgetClass);
	instance->AddToViewport();
	LobbyWidget = Cast<UNALobbyWidget>(instance);

	// instance = CreateWidget<UUserWidget>(GetWorld(), SessionListWidgetClass);
	// instance->AddToViewport();
	// LobbyWidget = Cast<UNa>(instance); 
}
