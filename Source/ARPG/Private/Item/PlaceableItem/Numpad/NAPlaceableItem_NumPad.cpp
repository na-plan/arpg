// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PlaceableItem/Numpad/NAPlaceableItem_NumPad.h"

#include "Item/PlaceableItem/Numpad/NANumpadWidget.h"

#include "Editor.h"
#include "Misc/NAWorldEventHandler.h"
#include "Subsystems/EditorActorSubsystem.h"

// Sets default values
ANAPlaceableItem_NumPad::ANAPlaceableItem_NumPad(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ANAPlaceableItem_NumPad::PostInitProperties()
{
	Super::PostInitProperties();
}

void ANAPlaceableItem_NumPad::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
}

void ANAPlaceableItem_NumPad::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ANAPlaceableItem_NumPad::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ANAPlaceableItem_NumPad::BeginInteract_Implementation(AActor* Interactor)
{
	Super::BeginInteract_Implementation(Interactor);

	// UNAWorldEventHandler::GetInstance()->RegisterEvent(TEXT("Test"), [this]()
	// {
	// 	ExecuteInteract_Wrapped();
	// });

	CachedWidget->SetVisibility(ESlateVisibility::Visible);
}

void ANAPlaceableItem_NumPad::EndInteract_Implementation(AActor* Interactor)
{
	Super::EndInteract_Implementation(Interactor);

	//UNAWorldEventHandler::GetInstance()->UnRegisterEvent(TEXT("Test"));
}

bool ANAPlaceableItem_NumPad::ExecuteInteract_Implementation(AActor* Interactor)
{
	return true;
}

// Called when the game starts or when spawned
void ANAPlaceableItem_NumPad::BeginPlay()
{
	Super::BeginPlay();
	
	UUserWidget* instance = CreateWidget<UUserWidget>(GetWorld(), NumpadWidget);
	if (!instance) return;

	instance->AddToViewport();
	instance->SetVisibility(ESlateVisibility::Hidden);

	CachedWidget = Cast<UNANumpadWidget>(instance);
}

void ANAPlaceableItem_NumPad::ExecuteInteract_Wrapped()
{
	if (TargetActor == nullptr) return;
	
	TargetActor->Execute_BeginInteract(this, this);
}

// Called every frame
void ANAPlaceableItem_NumPad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

