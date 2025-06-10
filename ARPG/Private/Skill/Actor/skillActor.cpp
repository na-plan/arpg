// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/Actor/skillActor.h"


// Sets default values
AskillActor::AskillActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AskillActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AskillActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

