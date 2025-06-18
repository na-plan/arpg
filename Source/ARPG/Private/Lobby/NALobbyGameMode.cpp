// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/NALobbyGameMode.h"

#include "CineCameraActor.h"
#include "NAGameStateBase.h"
#include "NAPlayerController.h"
#include "NAPlayerState.h"
#include "Components/DirectionalLightComponent.h"
#include "Item/ItemActor/NAItemActor.h"
#include "Kismet/GameplayStatics.h"
#include "Lobby/NALobbyWidget.h"

ANALobbyGameMode::ANALobbyGameMode()
{
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
	PlayerStateClass = ANAPlayerState::StaticClass();
	GameStateClass = ANAGameStateBase::StaticClass();
	PlayerControllerClass = ANAPlayerController::StaticClass();
	
	static ConstructorHelpers::FClassFinder<UUserWidget> widget(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/00_ProjectNA/01_Blueprint/01_Widget/Lobby/BP_NALobby.BP_NALobby_C'"));
	if (widget.Succeeded())
		LobbyWidgetClass = widget.Class;
}

void ANALobbyGameMode::BeginPlay()
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

	
}
//
// void ANALobbyGameMode::OnClick_SingleMode()
// {
// 	ChangeLevel(GetWorld(),TEXT("/Script/Engine.World'/Game/00_ProjectNA/02_Level/Level_NATestLevel.Level_NATestLevel'"));
// }
//
// void ANALobbyGameMode::OnClick_CoopMode()
// {
// }
// void ANALobbyGameMode::ChangeLevel(UObject* WorldContext, FString LevelName)
// {
// 	UGameplayStatics::OpenLevel(WorldContext, FName(LevelName));
//
// 	
// 	// 멀티
// 	//GetWorld()->ServerTravel(LevelName + TEXT("?listen"));;
// }
