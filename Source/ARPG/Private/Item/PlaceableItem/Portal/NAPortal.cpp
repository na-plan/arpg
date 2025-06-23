// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PlaceableItem/Portal/NAPortal.h"

#include "Abilities/GameplayAbilityTargetActor.h"


// Sets default values
ANAPortal::ANAPortal(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ANAPortal::PostInitProperties()
{
	Super::PostInitProperties();
}

void ANAPortal::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
}

void ANAPortal::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ANAPortal::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

bool ANAPortal::BeginInteract_Implementation(AActor* Interactor)
{
	return Super::BeginInteract_Implementation(Interactor);
}

bool ANAPortal::ExecuteInteract_Implementation(AActor* Interactor)
{
	TeleportToDest(Interactor);
	
	return Super::ExecuteInteract_Implementation(Interactor);
}

bool ANAPortal::EndInteract_Implementation(AActor* Interactor)
{
	return Super::EndInteract_Implementation(Interactor);
}

void ANAPortal::SetInteractionPhysicsEnabled(const bool bEnabled)
{
	Super::SetInteractionPhysicsEnabled(bEnabled);
}

// Called when the game starts or when spawned
void ANAPortal::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ANAPortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ANAPortal::TeleportToDest(AActor* InterActor)
{
	if (!InterActor) return;
	
	FTransform DestTf = DestActor->GetTransform();
	InterActor->SetActorLocationAndRotation(DestTf.GetLocation(), DestTf.GetRotation().GetNormalized(), false, nullptr, ETeleportType::ResetPhysics);
}

