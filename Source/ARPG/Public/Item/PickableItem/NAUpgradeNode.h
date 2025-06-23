// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemActor/NAPickableItemActor.h"
#include "Item/ItemDataStructs/NAUpgradeNodeDataStructs.h"
#include "NAUpgradeNode.generated.h"

UCLASS()
class ARPG_API ANAUpgradeNode : public ANAPickableItemActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANAUpgradeNode(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
