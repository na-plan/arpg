// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PlaceableItem/UpgradeBench/NAUpgradeBench.h"

#include "Item/PlaceableItem/UpgradeBench/NAUpgradeBenchComponent.h"


// Sets default values
ANAUpgradeBench::ANAUpgradeBench(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer
		.DoNotCreateDefaultSubobject(TEXT("ItemWidgetComponent")))
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateOptionalDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh(Static)"));
	if (ItemMesh)
	{
		bNeedItemMesh = true;
	}
	ItemWidgetComponent = CreateOptionalDefaultSubobject<UNAUpgradeBenchComponent>(TEXT("UpgradeBenchComponent"));
}

// Called every frame
void ANAUpgradeBench::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called when the game starts or when spawned
void ANAUpgradeBench::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ANAUpgradeBench::CanInteract_Implementation() const
{
	return Super::CanInteract_Implementation();
}

bool ANAUpgradeBench::IsOnInteract_Implementation() const
{
	return Super::IsOnInteract_Implementation();
}

bool ANAUpgradeBench::TryInteract_Implementation(AActor* Interactor)
{
	//return Super::TryInteract_Implementation(Interactor);
	return false;
}

bool ANAUpgradeBench::BeginInteract_Implementation(AActor* Interactor)
{
	return Super::BeginInteract_Implementation(Interactor);
}

bool ANAUpgradeBench::ExecuteInteract_Implementation(AActor* Interactor)
{
	return Super::ExecuteInteract_Implementation(Interactor);
}

bool ANAUpgradeBench::EndInteract_Implementation(AActor* Interactor)
{
	return Super::EndInteract_Implementation(Interactor);
}


