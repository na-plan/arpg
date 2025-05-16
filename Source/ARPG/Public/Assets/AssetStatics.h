#pragma once

#include "CoreMinimal.h"
#include "Interface/NAManagedAsset.h"

struct FAssetStatics
{
	static void RetrieveAsset(const TScriptInterface<INAManagedAsset>& Interface);
	static TSubclassOf<AActor> GetAssetClass(const UWorld* World, const FName& InAssetName);
};
