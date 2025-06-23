// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemActor/NAPickableItemActor.h"
#include "NAUpgradeNode.generated.h"

UCLASS()
class ARPG_API ANAUpgradeNode : public ANAPickableItemActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANAUpgradeNode(const FObjectInitializer& ObjectInitializer);
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

//======================================================================================================================
// Item Use Interface Implements
//======================================================================================================================
public:
	virtual bool CanUseItem(UNAItemData* InItemData, AActor* User) const override;
	virtual bool UseItem(UNAItemData* InItemData, AActor* User, int32& UsedAmount) const override;
};
