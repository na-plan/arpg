// Fill out your copyright notice in the Description page of Project Settings.


#include "Assets/GameInstance/NAAssetGameInstanceSubsystem.h"

#include "Assets/Interface/NAManagedAsset.h"
#include "Assets/TableRow/AssetTableRow.h"

DEFINE_LOG_CATEGORY(LogAssetGameInstance);

/*
void UNAAssetGameInstanceSubsystem::OnActorSpawned(AActor* InActor) const
{
	FetchAsset(InActor);
}

void UNAAssetGameInstanceSubsystem::OnPreWorldInitialization(UWorld* InWorld,
	const FWorldInitializationValues WorldInitializationValues)
{
	// 월드에 대해 구독, 게임월드 만을 대상으로 (-에디터)
	if (InWorld && InWorld->IsGameWorld())
	{
		FOnActorSpawned::FDelegate Delegate;
		Delegate.BindUObject( this, &UNAAssetGameInstanceSubsystem::OnActorSpawned );
		OnAssetSpawnedDelegate = InWorld->AddOnActorSpawnedHandler(Delegate);
	}
}

void UNAAssetGameInstanceSubsystem::OnPostWorldCleanup(UWorld* World, bool bArg, bool bCond)
{
	if ( OnAssetSpawnedDelegate.IsValid() && World->IsGameWorld() )
	{
		World->RemoveOnActorSpawnedHandler(OnAssetSpawnedDelegate);
	}
}
*/

void UNAAssetGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG( LogInit, Log, TEXT("%hs"), __FUNCTION__ )
	AssetTable = Cast<UDataTable>( StaticLoadObject( UDataTable::StaticClass(), nullptr, TEXT("/Script/Engine.DataTable'/Game/00_ProjectNA/04_DataTable/00_Assets/AssetsTable.AssetsTable'") ) );
	
	// 월드의 Actor 스폰을 잡기 위해서
	// FWorldDelegates::OnPreWorldInitialization.AddUObject(this, &UNAAssetGameInstanceSubsystem::OnPreWorldInitialization);
	// FWorldDelegates::OnPostWorldCleanup.AddUObject(this, &UNAAssetGameInstanceSubsystem::OnPostWorldCleanup);
}

void UNAAssetGameInstanceSubsystem::FetchAsset(UObject* InActor) const
{
	if (const TScriptInterface<INAManagedAsset> Interface(InActor); Interface)
	{
		if (Interface->GetAssetName().IsNone())
		{
			UE_LOG(LogAssetGameInstance, Warning, TEXT("Actor %s does not have asset name defined. ignore..."), *InActor->GetName());
		}
		else
		{
			UE_LOG(LogAssetGameInstance, Log, TEXT("Loading Asset: %s"), *Interface->GetAssetName().ToString());
			if (const FAssetTableRow* Row = AssetTable->FindRow<FAssetTableRow>(Interface->GetAssetName(), TEXT("AssetGameInstance Query")))
			{
				Interface->RetrieveAsset(Row->GetCDOAsset());
			}
		}
	}
}

TSubclassOf<AActor> UNAAssetGameInstanceSubsystem::GetAssetClass(const FName& InAssetName) const
{
	UE_LOG(LogAssetGameInstance, Log, TEXT("Loading Asset: %s"), *InAssetName.ToString());
	if (const FAssetTableRow* Row = AssetTable->FindRow<FAssetTableRow>(InAssetName, TEXT("AssetGameInstance Query")))
	{
		return Row->GetAssetClass();
	}
	else
	{
		ensureMsgf(Row, TEXT("Unable to find the asset table"));
	}

	return nullptr;
}
