// Fill out your copyright notice in the Description page of Project Settings.


#include "HP/Widget/NAHPWidget.h"

#include "NAPlayerState.h"
#include "Components/ProgressBar.h"

void UNAHPWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// CDO 초기화 방지 조건문
	if (GetWorld()->IsGameWorld())
	{
		HPBar->PercentDelegate.BindDynamic(this, &UNAHPWidget::GetHealthPercentage);
		HPBar->SynchronizeProperties();
	}
}

void UNAHPWidget::NativeDestruct()
{
	Super::NativeDestruct();

	// CDO 초기화 방지 조건문
	if (GetWorld()->IsGameWorld())
	{
		HPBar->PercentDelegate.Unbind();
		HPBar->SynchronizeProperties();
	}
}

float UNAHPWidget::GetHealthPercentage()
{
	if (const ANAPlayerState* PlayerState = GetPlayerContext().GetPlayerState<ANAPlayerState>();
		IsValid(PlayerState))
	{
		return PlayerState->GetHealth() / PlayerState->GetMaxHealth();
	}

	return 0.f;
}

