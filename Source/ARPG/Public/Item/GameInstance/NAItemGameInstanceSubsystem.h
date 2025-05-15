// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Item/NAItemData.h"
#include "NAItemGameInstanceSubsystem.generated.h"

class ANAItemInstance;
class UNAItemData;
/**
 * 
 */
UCLASS()
class ARPG_API UNAItemGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<UDataTable*> ItemDataTables;
	
	//UPROPERTY()
	//TMap<FName, ANAItemInstance*> ItemMap;

	UPROPERTY()
	//TMap<TSubclassOf<AActor>, ANAItemInstance*> ItemMetaData;
	//TMap<FName, UScriptStruct> ItemMetaDataMap;
	TMap<TSubclassOf<ANAItemInstance>, UScriptStruct> ItemMetaDataMap;

	UPROPERTY()
	//TMap<ANAItemInstance*, TSubclassOf<AActor>> ItemClassMap;
	TMap<FName, UNAItemData*> ItemDataMap;

	UPROPERTY()
	TArray<FObjectKey> ItemInstanceArray;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void UpdateItemDataMap(FName InItemDataID, UNAItemData* InItemData);
};
