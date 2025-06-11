// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NAItemWidget.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNAItemWidget : public UUserWidget
{
	GENERATED_BODY()

	friend class UNAItemWidgetComponent;
protected:
	// CreateWidget으로 위젯 인스턴스가 생성되고 나서 호출됨
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	void ReleaseItemWidget();
	void CollapseItemWidget();
	
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Item Widget")
	TObjectPtr<class UTextBlock> Item_Name;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Item Widget")
	TObjectPtr<class UImage> Item_Icon;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Item Widget")
	TObjectPtr<UTextBlock> Item_Type;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Item Widget")
	TObjectPtr<UTextBlock> Interaction_Name;
};
