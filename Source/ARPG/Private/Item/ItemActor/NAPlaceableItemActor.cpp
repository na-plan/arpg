#include "Item/ItemActor/NAPlaceableItemActor.h"

void ANAPlaceableItemActor::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
}

void ANAPlaceableItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ANAPlaceableItemActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ANAPlaceableItemActor::BeginInteract_Implementation(AActor* Interactor)
{
	Super::BeginInteract_Implementation(Interactor);
}

void ANAPlaceableItemActor::EndInteract_Implementation(AActor* Interactor)
{
	Super::EndInteract_Implementation(Interactor);
}

void ANAPlaceableItemActor::ExecuteInteract_Implementation(AActor* Interactor)
{
	Super::ExecuteInteract_Implementation(Interactor);

	// @TODO: 환경 오브젝트의 상호작용 로직 -> 문열기, 레버 당기기 등

	if (GEngine) {
		FString Log = TEXT("Placeable Item Instance executes interaction.");
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Emerald, *Log);
	}
}

void ANAPlaceableItemActor::BeginPlay()
{
	Super::BeginPlay();
}
