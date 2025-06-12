// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PlaceableItem/Numpad/NAPlaceableItem_NumPad.h"


// Sets default values
ANAPlaceableItem_NumPad::ANAPlaceableItem_NumPad()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ANAPlaceableItem_NumPad::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANAPlaceableItem_NumPad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

