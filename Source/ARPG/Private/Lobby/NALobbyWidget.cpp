// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/NALobbyWidget.h"
#include "Components/Button.h"
#include "Kismet/GamePlayStatics.h"
#include "Lobby/NALobbyGameState.h"

UNALobbyWidget::UNALobbyWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UNALobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!GetWorld()->IsGameWorld()) return;
	
	InitWidgets();
}

void UNALobbyWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UNALobbyWidget::InitWidgets()
{
	if (!Button_SinglePlay) return;
	if (!Button_CoopPlay) return;
	if (!Button_TestPlay) return;
	
	// 이벤트 바인딩
	Button_SinglePlay->OnClicked.AddUniqueDynamic(this, &ThisClass::OnClick_SinglePlay);
	Button_CoopPlay->OnClicked.AddUniqueDynamic(this, &ThisClass::OnClick_CoopPlay);
	Button_TestPlay->OnClicked.AddUniqueDynamic(this, &ThisClass::OnClick_TestPlay);
}

void UNALobbyWidget::OnClick_SinglePlay()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("NATutorial"));
}

void UNALobbyWidget::OnClick_CoopPlay()
{
	// todo: 매칭시작
	ANALobbyGameState* GameState = GetWorld()->GetGameStateChecked<ANALobbyGameState>();
	
}

void UNALobbyWidget::OnClick_TestPlay()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Script/Engine.World'/Game/ThirdPerson/Maps/ThirdPersonMap.ThirdPersonMap'"));
}
