// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Spawner/SpawnerController.h"

#include "Components/BoxComponent.h"

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
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);




}

// Called when the game starts or when spawned
void ASpawnerController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASpawnerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

