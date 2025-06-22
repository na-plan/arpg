// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemWidget/NAItemWidget.h"
#include "NAUpgradeBenchWidget.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNAUpgradeBenchWidget : public UNAItemWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	virtual void InitUpgradeBenchWidget(class UNAUpgradeBenchComponent* OwningComp);
	
	virtual void ReleaseItemWidget() override;
	virtual void CollapseItemWidget() override;

protected:
	virtual void OnItemWidgetReleased() override;
	virtual void OnItemWidgetCollapsed() override;
};
