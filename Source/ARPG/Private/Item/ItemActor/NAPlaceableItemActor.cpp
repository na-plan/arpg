#include "Item/ItemActor/NAPlaceableItemActor.h"

ANAPlaceableItemActor::ANAPlaceableItemActor(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

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
	// 상호작용 시작 그리고 실제 실행 대기 ( 레버 당기는 시간, 활성화 대기 시간... )
}

void ANAPlaceableItemActor::EndInteract_Implementation(AActor* Interactor)
{
	Super::EndInteract_Implementation(Interactor);
	// 상호작용이 끝난 후 처리
}

bool ANAPlaceableItemActor::ExecuteInteract_Implementation(AActor* Interactor)
{
	Super::ExecuteInteract_Implementation(Interactor);

	// @TODO: 환경 오브젝트의 상호작용 로직 -> 문열기, 레버 당기기 등

	// if (GEngine) {
	// 	FString Log = TEXT("Placeable Item Instance executes interaction.");
	// 	GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Emerald, *Log);
	// }
	return true;
}

void ANAPlaceableItemActor::BeginPlay()
{
	Super::BeginPlay();
}
