// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnerController.generated.h"

class AMonsterSpawner;
class UBoxComponent;

UCLASS()
class ARPG_API ASpawnerController : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<class AMonsterSpawner*> MonsterSPawner;
	

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* CollisionComponent;

public:	
	// Sets default values for this actor's properties
	ASpawnerController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
