// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PlaceableItem/NAButtonSwitch.h"


// Sets default values
ANAButtonSwitch::ANAButtonSwitch(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TargetToActive = CreateDefaultSubobject<ANAPlaceableItemActor>("TargetToActive");
}

void ANAButtonSwitch::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
}

void ANAButtonSwitch::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ANAButtonSwitch::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ANAButtonSwitch::BeginInteract_Implementation(AActor* Interactor)
{
	Super::BeginInteract_Implementation(Interactor);
}

void ANAButtonSwitch::EndInteract_Implementation(AActor* Interactor)
{
	Super::EndInteract_Implementation(Interactor);
}

bool ANAButtonSwitch::ExecuteInteract_Implementation(AActor* Interactor)
{
	Super::ExecuteInteract_Implementation(Interactor);

	return bIsActive = true;
}

// Called when the game starts or when spawned
void ANAButtonSwitch::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANAButtonSwitch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

