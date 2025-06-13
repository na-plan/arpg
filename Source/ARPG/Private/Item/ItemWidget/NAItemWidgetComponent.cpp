// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemWidget/NAItemWidgetComponent.h"

#include "Item/ItemActor/NAPickableItemActor.h"
#include "Item/ItemActor/NAPlaceableItemActor.h"
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
	SetWindowFocusable(false);
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
			UClass* LoadedClass = LoadClass<UNAItemWidget>(nullptr, TEXT("/Game/00_ProjectNA/ItemTest/ItemWidget/BP_NAPickableItemWidget.BP_NAPickableItemWidget_C"));
			if (LoadedClass)
			{
				SetWidgetClass(LoadedClass);
				bIsPickableItem = true;
			}
		}
		else if (GetOwner()->GetClass()->IsChildOf<ANAPlaceableItemActor>())
		{
			UClass* LoadedClass = LoadClass<UNAItemWidget>(nullptr, TEXT("/Game/00_ProjectNA/ItemTest/ItemWidget/BP_NAPlaceableItemWidget.BP_NAPlaceableItemWidget_C"));
			if (LoadedClass)
			{
				SetWidgetClass(LoadedClass);
				bIsPlaceableItem = true;
			}
		}
		
		if (bIsPickableItem)
		{
			SetRelativeRotation(FRotator(0.0f, 180.0f,0.0f ));
			SetDrawSize(FVector2D(340, 410));
			SetRelativeScale3D(FVector(0.4f));
		}
		else if (bIsPlaceableItem)
		{
			SetRelativeRotation(FRotator(0.0f, 180.0f, 0.f));
			SetDrawSize(FVector2D(260, 50));
		}
	}
}

// Called when the game starts
void UNAItemWidgetComponent::BeginPlay()
{
	Super::BeginPlay();

	SetVisibility(false);
	Deactivate();
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

void UNAItemWidgetComponent::InitWidget()
{
	Super::InitWidget();
	
	if (GetWorld()->IsPreviewWorld()) return;
	if (!GetItemDataFromOwner()) return;
	
	if (GetItemWidget())
	{
		GetItemWidget()->InitItemWidget(this, GetItemDataFromOwner());
	}
}

UNAItemData* UNAItemWidgetComponent::GetItemDataFromOwner() const
{
	if (ANAItemActor* ItemActor = Cast<ANAItemActor>(GetOwner()))
	{
		return ItemActor->GetItemData();
	}
	return nullptr;
}

void UNAItemWidgetComponent::ReleaseItemWidgetPopup()
{
	if (GetWorld()->IsPreviewWorld()) return;
	if (!GetItemDataFromOwner()) return;

	if (GetOwner()->GetClass()->IsChildOf<ANAPickableItemActor>())
	{
		bFaceCamera = true;
		FaceCamera();
	}
	GetItemWidget()->ReleaseItemWidget();
}

void UNAItemWidgetComponent::CollapseItemWidgetPopup()
{
	if (GetWorld()->IsPreviewWorld()) return;
	if (!GetItemDataFromOwner()) return;

	if (GetOwner()->GetClass()->IsChildOf<ANAPickableItemActor>())
	{
		bFaceCamera = false;
	}
	GetItemWidget()->CollapseItemWidget();
}

class UNAItemWidget* UNAItemWidgetComponent::GetItemWidget() const
{
	return Cast<UNAItemWidget>(GetWidget());
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

