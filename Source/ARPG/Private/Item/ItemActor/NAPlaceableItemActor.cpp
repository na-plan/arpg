#include "Item/ItemActor/NAPlaceableItemActor.h"

#include "Components/ShapeComponent.h"

ANAPlaceableItemActor::ANAPlaceableItemActor(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer
		.DoNotCreateDefaultSubobject(TEXT("ItemMesh(Static)")))
{
	bNeedItemMesh = false;
}

void ANAPlaceableItemActor::PostInitProperties()
{
	Super::PostInitProperties();
}

void ANAPlaceableItemActor::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
}

void ANAPlaceableItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (ItemCollision)
	{
		ItemCollision->SetSimulatePhysics(false);
		ItemCollision->SetGenerateOverlapEvents(false);
		ItemCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ANAPlaceableItemActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

bool ANAPlaceableItemActor::ExecuteInteract_Implementation(AActor* Interactor)
{
	Super::ExecuteInteract_Implementation(Interactor);

	// @TODO: 환경 오브젝트의 상호작용 로직 -> 문열기, 레버 당기기 등

	if (GEngine) {
		FString Log = TEXT("Placeable Item Instance executes interaction.");
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Emerald, *Log);
	}
	return true;
}

bool ANAPlaceableItemActor::BeginInteract_Implementation(AActor* Interactor)
{
	return Super::BeginInteract_Implementation(Interactor);
	// 상호작용 시작 그리고 실제 실행 대기 ( 레버 당기는 시간, 활성화 대기 시간... )
}

bool ANAPlaceableItemActor::EndInteract_Implementation(AActor* Interactor)
{
	return Super::EndInteract_Implementation(Interactor);
	// 상호작용이 끝난 후 처리
}

void ANAPlaceableItemActor::BeginPlay()
{
	Super::BeginPlay();

	if (ItemCollision)
	{
		ItemCollision->SetSimulatePhysics(false);
		ItemCollision->SetGenerateOverlapEvents(false);
		ItemCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
