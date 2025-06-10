#include "Assets/AssetStatics.h"

#include "Assets/GameInstance/NAAssetGameInstanceSubsystem.h"

void FAssetStatics::RetrieveAsset(const TScriptInterface<INAManagedAsset>& Interface)
{
	const UWorld* World = Interface.GetObject()->GetWorld();
	check(World);

	if (const UNAAssetGameInstanceSubsystem* AssetGameInstanceSubsystem = World->GetGameInstance()->GetSubsystem<UNAAssetGameInstanceSubsystem>())
	{
		AssetGameInstanceSubsystem->FetchAsset(Interface.GetObject());
	}
}

TSubclassOf<AActor> FAssetStatics::GetAssetClass(const UWorld* World, const FName& InAssetName)
{
	check(World);

	if (const UNAAssetGameInstanceSubsystem* AssetGameInstanceSubsystem = World->GetGameInstance()->GetSubsystem<UNAAssetGameInstanceSubsystem>())
	{
		return AssetGameInstanceSubsystem->GetAssetClass(InAssetName);
	}
	else
	{
		ensureMsgf(AssetGameInstanceSubsystem, TEXT("Unable to load the asset"));
	}

	return nullptr;
}
