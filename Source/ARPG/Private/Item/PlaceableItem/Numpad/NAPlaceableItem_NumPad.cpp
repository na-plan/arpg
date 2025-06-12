// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PlaceableItem/Numpad/NAPlaceableItem_NumPad.h"

#include "Item/PlaceableItem/Numpad/NANumpadWidget.h"

#include "Editor.h"
#include "Subsystems/EditorActorSubsystem.h"

// Sets default values
ANAPlaceableItem_NumPad::ANAPlaceableItem_NumPad(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FClassFinder<UUserWidget> foundWidget(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/00_ProjectNA/01_Blueprint/01_Widget/InGame/BP_NANumPad.BP_NANumPad_C'"));
	if (foundWidget.Succeeded())
		NumpadWidget = foundWidget.Class;
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
}

void ANAPlaceableItem_NumPad::EndInteract_Implementation(AActor* Interactor)
{
	Super::EndInteract_Implementation(Interactor);
}

bool ANAPlaceableItem_NumPad::ExecuteInteract_Implementation(AActor* Interactor)
{
	
	
	return true;
}

void ANAPlaceableItem_NumPad::SelectActorInEditor(AActor* InActor)
{
#if WITH_EDITOR

		if (TargetActor && GEditor)
		{
			GEditor->SelectNone(false, true, false);                      // 기존 선택 해제
			GEditor->SelectActor(InActor, true, true, true);         // 대상 액터 선택
		}
#endif
}

// Called when the game starts or when spawned
void ANAPlaceableItem_NumPad::BeginPlay()
{
	Super::BeginPlay();
	
	 UUserWidget* instance = CreateWidget<UUserWidget>(GetWorld(), NumpadWidget);
	instance->AddToViewport();
	//Widget->SetVisibility(ESlateVisibility::Hidden);
}

// Called every frame
void ANAPlaceableItem_NumPad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

