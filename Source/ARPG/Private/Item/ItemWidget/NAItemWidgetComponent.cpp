// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemWidget/NAItemWidgetComponent.h"

#include "Item/ItemActor/NAPickableItemActor.h"
#include "Item/ItemWidget/NAItemWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UNAItemWidgetComponent::UNAItemWidgetComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

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
	SetGeometryMode(EWidgetGeometryMode::Plane);
	OpacityFromTexture = 1.f;
	SetBlendMode(EWidgetBlendMode::Masked);
}

void UNAItemWidgetComponent::PostInitProperties()
{
	Super::PostInitProperties();

	if (GetOwner() && GetWidgetClass() == nullptr)
	{
		bool bIsPickableItem = false;
		bool bIsPlaceableItem = false;
		if (GetOwner()->GetClass()->IsChildOf<ANAPickableItemActor>())
		{
			UClass* LoadedClass = LoadClass<UNAItemWidget>(nullptr, TEXT("/Game/00_ProjectNA/ItemTest/ItemWidget/BP_NAItemWidget.BP_NAItemWidget_C"));
			if (LoadedClass)
			{
				SetWidgetClass(LoadedClass);
				bIsPickableItem = true;
			}
		}
		else if (GetOwner()->GetClass()->IsChildOf<ANAPickableItemActor>())
		{
			// @TODO: PlacableItemActor 전용 위젯 클래스 만들기
			// UClass* LoadedClass = LoadClass<UNAItemWidget>(nullptr, TEXT("/Game/00_ProjectNA/ItemTest/ItemWidget/BP_NAItemWidget.BP_NAItemWidget_C"));
			// if (LoadedClass)
			// {
			// 	SetWidgetClass(LoadedClass);
			// 	bIsPlaceableItem = true;
			// }
		}
		
		if (bIsPickableItem)
		{
			SetDrawSize(FVector2D(280, 350));
		}
		else if (bIsPlaceableItem)
		{
			// @TODO: Placeable Item 전용 위젯 드로 사이즈 구하기
			SetDrawSize(FVector2D(680, 470));
		}
	}
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

	if (bFaceCamera)
	{
		FaceCamera();
	}
}

void UNAItemWidgetComponent::FaceCamera()
{
	if (!bFaceCamera) return;
	
	int32 PlayerIndex = GetOwnerPlayer()->GetLocalPlayerIndex();
	APlayerCameraManager* CM = UGameplayStatics::GetPlayerCameraManager(this, PlayerIndex);
	FVector ToCamera = CM->GetCameraLocation() - GetComponentLocation();
	FRotator LookRot = UKismetMathLibrary::MakeRotFromX(ToCamera);
	LookRot.Pitch = 0.f;
	LookRot.Roll = 0.f;
	SetWorldRotation(LookRot);
}

