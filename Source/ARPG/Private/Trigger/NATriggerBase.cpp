// Fill out your copyright notice in the Description page of Project Settings.


#include "Trigger/NATriggerBase.h"

#include "Components/ShapeComponent.h"


// Sets default values
ANATriggerBase::ANATriggerBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ANATriggerBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANATriggerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

