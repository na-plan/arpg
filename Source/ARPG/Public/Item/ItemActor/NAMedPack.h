// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NAPickableItemActor.h"
#include "NAMedPack.generated.h"

UCLASS()
class ARPG_API ANAMedPack : public ANAPickableItemActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANAMedPack(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
