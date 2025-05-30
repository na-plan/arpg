// Copyright Epic Games, Inc. All Rights Reserved.

#include "NAGameMode.h"

#include "NACharacter.h"
#include "NAGameStateBase.h"
#include "NAInGameHUD.h"
#include "NAPlayerController.h"
#include "NAPlayerState.h"
#include "Assets/Interface/NAManagedAsset.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ANAGameMode::ANAGameMode()
{
	DefaultPawnClass = ANACharacter::StaticClass();
	PlayerStateClass = ANAPlayerState::StaticClass();
	GameStateClass = ANAGameStateBase::StaticClass();
	PlayerControllerClass = ANAPlayerController::StaticClass();
	HUDClass = ANAInGameHUD::StaticClass();
}

APawn* ANAGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save default player pawns into a map
	SpawnInfo.CustomPreSpawnInitalization = [NewPlayer](AActor* InActor)
	{
		// 기존의 SpawnDefaultPawnAtTransform 구현부 복사 붙여넣기 + PreSpawn 함수 추가
		// PlayerState가 먼저 만들어지고 (캐릭터 에셋이 부여되고), 그 다음에 Pawn이 생성됨
		if (const ANAPlayerController* PlayerController = Cast<ANAPlayerController>(NewPlayer))
		{
			if (const ANAPlayerState* PlayerState = PlayerController->GetPlayerState<ANAPlayerState>())
			{
				if (const TScriptInterface<INAManagedAsset> Interface = InActor)
				{
					Interface->SetAssetName(PlayerState->GetPossessAssetName());
				}
			}
		}
	};
	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);
	APawn* ResultPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo);
	if (!ResultPawn)
	{
		UE_LOG(LogGameMode, Warning, TEXT("SpawnDefaultPawnAtTransform: Couldn't spawn Pawn of type %s at %s"), *GetNameSafe(PawnClass), *SpawnTransform.ToHumanReadableString());
	}
	
	return ResultPawn;
}
