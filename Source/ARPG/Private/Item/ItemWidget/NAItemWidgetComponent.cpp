// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemWidget/NAItemWidgetComponent.h"
#include "Item/ItemWidget/NAItemWidget.h"


// Sets default values for this component's properties
UNAItemWidgetComponent::UNAItemWidgetComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FClassFinder<UNAItemWidget> ItemWidgetClass(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/00_ProjectNA/ItemTest/ItemWidget/BP_NAItemWidget.BP_NAItemWidget_C'"));
	check(ItemWidgetClass.Class);
	SetWidgetClass(ItemWidgetClass.Class);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetEnableGravity(false);
	CanCharacterStepUpOn = ECB_No;
	SetSimulatePhysics(false);
	bApplyImpulseOnDamage = false;
	bReplicatePhysicsToAutonomousProxy = false;
	BodyInstance.SetInertiaConditioningEnabled(false);
	BodyInstance.bGenerateWakeEvents = false;
	BodyInstance.bUpdateMassWhenScaleChanges = false;

	SetWidgetSpace(EWidgetSpace::World);
	SetDrawSize(FVector2D(680, 470));
	SetGeometryMode(EWidgetGeometryMode::Plane);
	OpacityFromTexture = 1.f;
	SetBlendMode(EWidgetBlendMode::Masked);
}


// Called when the game starts
void UNAItemWidgetComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UNAItemWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

