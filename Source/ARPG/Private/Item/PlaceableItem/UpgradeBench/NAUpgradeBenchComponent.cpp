// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PlaceableItem/UpgradeBench/NAUpgradeBenchComponent.h"

#include "Item/PlaceableItem/UpgradeBench/NAUpgradeBench.h"
#include "Item/PlaceableItem/UpgradeBench/NAUpgradeBenchWidget.h"


// Sets default values for this component's properties
UNAUpgradeBenchComponent::UNAUpgradeBenchComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	static const FSoftClassPath PlaceableItemWidgetClassPath(TEXT("/Game/00_ProjectNA/ItemTest/UpgradeBench/BP_NAUpgradeBenchWidget.BP_NAUpgradeBenchWidget_C"));
	PlaceableItemWidgetClassRef = PlaceableItemWidgetClassPath;
}

// Called when the game starts
void UNAUpgradeBenchComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UNAUpgradeBenchComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UNAUpgradeBenchComponent::InitWidget()
{
	UWidgetComponent::InitWidget();

	if (IsValid(GetUpgradeBenchWidget()))
	{
		GetUpgradeBenchWidget()->InitUpgradeBenchWidget(this);
	}
}

void UNAUpgradeBenchComponent::ReleaseItemWidgetPopup()
{
	if (GetWorld()->IsPreviewWorld()) return;
	if (!GetUpgradeBenchWidget()) return;

	GetUpgradeBenchWidget()->ReleaseItemWidget();
}

void UNAUpgradeBenchComponent::CollapseItemWidgetPopup()
{
	if (GetWorld()->IsPreviewWorld()) return;
	if (!GetUpgradeBenchWidget()) return;
	
	GetUpgradeBenchWidget()->CollapseItemWidget();
}

class UNAUpgradeBenchWidget* UNAUpgradeBenchComponent::GetUpgradeBenchWidget() const
{
	return Cast<UNAUpgradeBenchWidget>(GetWidget());
}

void UNAUpgradeBenchComponent::InitItemWidgetClass()
{
	ensureAlways(GetOwner() && GetOwner()->IsA<ANAUpgradeBench>());
	
	Super::InitItemWidgetClass();
	OwningUpgradeBenchActor = Cast<ANAUpgradeBench>(GetOwner());
}

void UNAUpgradeBenchComponent::InitItemWidgetTransform()
{
	// @TODO: 위젯 트랜스폼 설정
}

