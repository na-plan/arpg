// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "NAItemWidgetComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_API UNAItemWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNAItemWidgetComponent(const FObjectInitializer& ObjectInitializer);
	virtual void PostInitProperties() override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	virtual void InitWidget() override;

	class UNAItemData* GetItemDataFromOwner() const;
	
	void ReleaseItemWidgetPopup();
	void CollapseItemWidgetPopup();

	class UNAItemWidget* GetItemWidget() const;

	void SetItemInteractionName(const FString& NewString) const;

	void SetEnableUpdateTransform(const bool bEnable);
	
protected:
	void UpdateTransformFacingCamera();
	void UpdateTransformFacingCharacter();

private:
	uint8 bUpdateTransformFacingCamera : 1 = false;

	// Yaw: 설정 반영 안됨(카메라 빌보드 처리)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FRotator PickableWidgetRelativeRotation = FRotator::ZeroRotator;
	
	// X: 앞/뒤  Y: 좌/우  Z: 상/하
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FVector PickableWidgetRelativeOffset = FVector::ZeroVector;

	
	uint8 bUpdateTransformFacingCharacter : 1 = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FTransform PlaceableWidgetRelativeForwardTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FTransform PlaceableWidgetRelativeBackwardTransform = FTransform::Identity;
};
