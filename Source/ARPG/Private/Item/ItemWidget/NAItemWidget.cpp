// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemWidget/NAItemWidget.h"
#include "Item/ItemWidget/NAItemWidgetComponent.h"

void UNAItemWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UNAItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UNAItemWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UNAItemWidget::ReleaseItemWidget()
{
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UNAItemWidget::CollapseItemWidget()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
