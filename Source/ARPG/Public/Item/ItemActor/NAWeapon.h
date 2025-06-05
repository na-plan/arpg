// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NAPickableItemActor.h"
#include "GameFramework/Actor.h"
#include "NAWeapon.generated.h"

class UNAMontageCombatComponent;

UCLASS()
class ARPG_API ANAWeapon : public ANAPickableItemActor
{
	GENERATED_BODY()

	friend class UNAItemEngineSubsystem;
	friend struct FCombatUpdatePredication;

	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess="true"))
	UNAMontageCombatComponent* CombatComponent;
	
public:
	// Sets default values for this actor's properties
	ANAWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick( float DeltaTime ) override;
};
