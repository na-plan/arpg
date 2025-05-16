// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterSpawner.generated.h"

UCLASS()
class ARPG_API AMonsterSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMonsterSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class", meta = (AllowPrivateAccess = "true"))
	FName AssetName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timer", meta=(AllowPrivateAccess="true"))
	float LastSpawnTime;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SpawnMonster();
	
};
