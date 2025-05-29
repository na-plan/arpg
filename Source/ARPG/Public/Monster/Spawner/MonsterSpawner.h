// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterSpawner.generated.h"

class AMonsterBase;

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

#if WITH_EDITOR
protected:
	//Editor Proxy
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
	//Duplacte In Editor
	virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;
	virtual void PostLoad() override;
	virtual void PostLoadSubobjects(FObjectInstancingGraph* OuterInstanceGraph) override;
	AActor* PreviewActor;
#endif
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class", meta = (AllowPrivateAccess = "true"))
	FName AssetName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timer", meta=(AllowPrivateAccess="true"))
	float LastSpawnTime;
	bool IsSpawn = false;
	bool IsSpawned = false;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SpawnMonster(bool Spawncheck);

	bool GetSpawning() { return IsSpawn; }
	bool SetSpawning(bool SetSpawn) { return IsSpawn= SetSpawn; }

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> PreviewSpawnTarget;

	UPROPERTY(EditAnywhere)
	bool SpawnOneTime = true;
	UPROPERTY(EditAnywhere)
	float SpawnTime = 10;

};
