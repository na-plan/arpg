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
	//Trigger에 닿으면 Spawn 할 대상
	UPROPERTY(EditAnywhere)
	TArray<AMonsterSpawner*> SpawnMonsterSPawner;

	//Trigger에 닿으면 dispawn 할 대상
	UPROPERTY(EditAnywhere)
	TArray<AMonsterSpawner*> DispawnMonsterSPawner;
	

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

	//Overlap Trigger Setting중
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Trigger를 지나지 않으면 spawn을 하지 않도록 하는 역할
	void DispawnSpawner();
	// Trigger 지나면 가지고있는 Trigger 들을 활성화 하는 역할
	void TriggOnSpawner();
	
};
