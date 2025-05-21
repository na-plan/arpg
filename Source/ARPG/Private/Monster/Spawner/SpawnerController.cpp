// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Spawner/SpawnerController.h"

#include "Components/BoxComponent.h"
#include "Monster/Spawner/MonsterSpawner.h"
#include "NACharacter.h"

// Sets default values
ASpawnerController::ASpawnerController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	//Trigger 만드는 과정
	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetupAttachment(RootComponent);
	//Overlap All
	//CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);

	//Overlap 되었을때
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBeginOverlap);
	//TriggOnSpawner를 호출해서 spawn하도록 하고 나갔을때 dispawn하도록 ㄱㄱ

}

// Called when the game starts or when spawned
void ASpawnerController::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASpawnerController::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//Player 가 overlap 되면
	if (ANACharacter* Player = Cast<ANACharacter>(OtherActor))
	{

		if (DispawnMonsterSPawner.Num() > 0)
		{
			//Dispawn할거 Dispawn
			DispawnSpawner();
		}

		if (SpawnMonsterSPawner.Num() > 0)
		{
			//Spawn할거 Spawn
			TriggOnSpawner();
		}
	}
	
}

// Called every frame
void ASpawnerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASpawnerController::DispawnSpawner()
{
	for (auto& Element : DispawnMonsterSPawner)
	{
		Element->SetSpawning(false);
	}
}

void ASpawnerController::TriggOnSpawner()
{
	
	for (auto& Element : SpawnMonsterSPawner)
	{
		Element->SetSpawning(true);
	}
}

