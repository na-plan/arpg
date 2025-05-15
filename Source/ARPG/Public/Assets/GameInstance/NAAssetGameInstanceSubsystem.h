// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NAAssetGameInstanceSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAssetGameInstance, Log, All);

/**
 * 
 */
UCLASS()
class ARPG_API UNAAssetGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UFUNCTION()
	void OnActorSpawned(AActor* InActor) const;

	void OnPreWorldInitialization(UWorld* InWorld, const FWorldInitializationValues WorldInitializationValues);

	void OnPostWorldCleanup(UWorld* World, bool bArg, bool bCond);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true", RequiredAssetDataTags="RowStructure=/Script/ARPG.AssetTableRow"))
	UDataTable* AssetTable;

	FDelegateHandle OnAssetSpawnedDelegate;
	
protected:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
public:
	UFUNCTION()
	void FetchAsset(AActor* InActor) const;
};
