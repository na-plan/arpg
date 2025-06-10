// Fill out your copyright notice in the Description page of Project Settings.


#include "NAGameStateBase.h"

#include "NAPlayerState.h"

void ANAGameStateBase::AddPlayerState(APlayerState* PlayerState)
{
	const int32 Index = PlayerArray.Num() % 2;
	Super::AddPlayerState(PlayerState);

	// 임시 플레이어 에셋 지정..
	if (ANAPlayerState* CastedPlayerState = Cast<ANAPlayerState>(PlayerState))
	{
		const FName CharacterAssetNames[]  = { "Male", "Female" };
		CastedPlayerState->SetPossessAssetName(CharacterAssetNames[Index]);
	}
}
