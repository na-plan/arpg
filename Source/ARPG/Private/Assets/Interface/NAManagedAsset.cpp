// Fill out your copyright notice in the Description page of Project Settings.


#include "Assets/Interface/NAManagedAsset.h"

#include "Assets/GameInstance/NAAssetGameInstanceSubsystem.h"


// Add default functionality here for any IAssetManaged functions that are not pure virtual.

void INAManagedAsset::RetrieveAssetIfAssetNameChanged(const FName& PreviousAssetName)
{
	// 이전의 에셋 이름과 현재 에셋 이름이 다르면 에셋을 가져오는 함수를 호출함
	if (PreviousAssetName != GetAssetName())
	{
		AActor* Actor = Cast<AActor>(_getUObject());
		ensure(Actor);

		const UWorld* World = Actor->GetWorld();
		ensure(World);

		if (const UNAAssetGameInstanceSubsystem* AssetGameInstanceSubsystem =  World->GetGameInstance()->GetSubsystem<UNAAssetGameInstanceSubsystem>())
		{
			AssetGameInstanceSubsystem->FetchAsset(Actor);
		}
	}
}

void INAManagedAsset::SetAssetName(const FName& InAssetName)
{
	const FName PreviousName = GetAssetName();
	SetAssetNameDerivedImplementation(InAssetName);
	RetrieveAssetIfAssetNameChanged(PreviousName);
}
