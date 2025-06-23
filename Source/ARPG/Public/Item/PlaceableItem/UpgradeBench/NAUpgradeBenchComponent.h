// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemWidget/NAItemWidgetComponent.h"
#include "NAUpgradeBenchComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_API UNAUpgradeBenchComponent : public UNAItemWidgetComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNAUpgradeBenchComponent(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	virtual void InitWidget() override;

	class UNAUpgradeBenchWidget* GetUpgradeBenchWidget() const;

	virtual void ReleaseItemWidgetPopup() override;
	virtual void CollapseItemWidgetPopup() override;

protected:
	virtual void InitItemWidgetClass() override;
	virtual void InitItemWidgetTransform() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<class ANAUpgradeBench> OwningUpgradeBenchActor;
};
