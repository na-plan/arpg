// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PlaceableItem/UpgradeBench/Widget/NAUpgradeBenchWidget.h"

#include "Item/PlaceableItem/UpgradeBench/NAUpgradeBenchComponent.h"


void UNAUpgradeBenchWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UNAUpgradeBenchWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UNAUpgradeBenchWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UNAUpgradeBenchWidget::InitUpgradeBenchWidget(class UNAUpgradeBenchComponent* OwningComp)
{
	check(OwningComp);

	OwningItemWidgetComponent = OwningComp;
}

void UNAUpgradeBenchWidget::ReleaseItemWidget()
{
	Super::ReleaseItemWidget();
	
}

void UNAUpgradeBenchWidget::CollapseItemWidget()
{
	Super::CollapseItemWidget();
	
}

void UNAUpgradeBenchWidget::OnItemWidgetReleased()
{
	Super::OnItemWidgetReleased();
	if (bReleaseItemWidget)
	{
	}
	else
	{
		
	}
}

void UNAUpgradeBenchWidget::OnItemWidgetCollapsed()
{
	Super::OnItemWidgetCollapsed();
	if (bReleaseItemWidget)
	{
	}
	else
	{
		
	}
}
