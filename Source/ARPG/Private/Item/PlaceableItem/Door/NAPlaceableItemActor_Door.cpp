// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PlaceableItem/Door/NAPlaceableItemActor_Door.h"
#include "Algo/Find.h"



// Sets default values
ANAPlaceableItemActor_Door::ANAPlaceableItemActor_Door(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
}

void ANAPlaceableItemActor_Door::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
}

void ANAPlaceableItemActor_Door::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ANAPlaceableItemActor_Door::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ANAPlaceableItemActor_Door::BeginInteract_Implementation(AActor* Interactor)
{
	Super::BeginInteract_Implementation(Interactor);

	InitInteraction(Interactor);
	TickInteraction.BindUObject(this, &ThisClass::ExecuteInteract_Implementation);
}

void ANAPlaceableItemActor_Door::EndInteract_Implementation(AActor* Interactor)
{
	Super::EndInteract_Implementation(Interactor);

	InitInteraction(nullptr);
	TickInteraction.Unbind();
}

bool ANAPlaceableItemActor_Door::ExecuteInteract_Implementation(AActor* Interactor)
{
	Super::ExecuteInteract_Implementation(Interactor);

	ActiveDoor();

	return true;
}

// Called when the game starts or when spawned
void ANAPlaceableItemActor_Door::BeginPlay()
{
	Super::BeginPlay();

	TArray<UStaticMeshComponent*> TargetComponents; 
	GetComponents(TargetComponents,true);

	for (UStaticMeshComponent* Comp : TargetComponents)
	{
		if (Comp->GetName() == "Door01")
			Door1 = Comp;
		if (Comp->GetName() == "Door02")
			Door2 = Comp;
	}
	
	OriginTF1 = Door1->GetComponentTransform();
	OriginTF2 = Door2->GetComponentTransform();

	PivotTF = RootComponent->GetRelativeTransform();
}

// Called every frame
void ANAPlaceableItemActor_Door::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsOnInteract)
	{
		CurrentTime += DeltaTime;
		TickInteraction.Execute(CurrentInteractingActor);
	}
}

void ANAPlaceableItemActor_Door::InitInteraction(AActor* Interactor = nullptr)
{
	CurrentInteractingActor = Interactor;
}

void ANAPlaceableItemActor_Door::ActiveDoor()
{
	if (CurrentTime > Duration) return;
	
	FVector OriginPos1 = OriginTF1.GetLocation();
	FVector OriginPos2 = OriginTF2.GetLocation();

	FVector Direction = DoorType == EDoorType::UpDown ? PivotTF.GetRotation().GetUpVector() : PivotTF.GetRotation().GetRightVector();
						
	FVector TargetPos1 = OriginTF1.GetLocation() + Direction * MoveDist;
	FVector TargetPos2 = OriginTF2.GetLocation() - Direction * MoveDist;

	float Alpha = CurrentTime / Duration;
	
	FVector MovingPos1 = FMath::InterpEaseIn(OriginPos1, TargetPos1, Alpha, 8.f);
	FVector MovingPos2 = FMath::InterpEaseIn(OriginPos2, TargetPos2, Alpha, 8.f);

	Door1->SetWorldLocation(MovingPos1);
	Door2->SetWorldLocation(MovingPos2);
}

