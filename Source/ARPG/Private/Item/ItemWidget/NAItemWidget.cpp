// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemWidget/NAItemWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Item/ItemWidget/NAItemWidgetComponent.h"
#include "Item/ItemData/NAItemData.h"
#include "Misc/StringUtils.h"

void UNAItemWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UNAItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(false);
	ForceLayoutPrepass();
	SetVisibility(ESlateVisibility::Collapsed);
}

void UNAItemWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UNAItemWidget::ReleaseItemWidget()
{
	if (!OwningItemWidgetComponent.IsValid()) return;
	
	bReleaseItemWidget = true;
	SetIsEnabled(true);
	if (Interaction_Name && bIsToggleAction)
	{
		SwapToggleActionText(ToggleActionText);
		Interaction_Name->SetText(FText::FromString(ToggleActionText));
		
	}
	if (Widget_Appear)
	{
		PlayAnimationForward(Widget_Appear, 1.5f);
	}
}

void UNAItemWidget::OnItemWidgetReleased()
{
	if (bReleaseItemWidget)
	{
		if (Widget_VisibleLoop)
		{
			PlayAnimation(Widget_VisibleLoop, 0.f, 0);
		}
	}
	else
	{
		if (Widget_VisibleLoop)
		{
			StopAnimation(Widget_VisibleLoop);
		}
	}
}

void UNAItemWidget::CollapseItemWidget()
{
	if (!OwningItemWidgetComponent.IsValid()) return;
	
	bReleaseItemWidget = false;
	SetIsEnabled(false);
	if (Widget_Appear)
	{
		PlayAnimationReverse(Widget_Appear, 1.8f);
	}
}

void UNAItemWidget::OnItemWidgetCollapsed()
{
	if (bReleaseItemWidget)
	{
		OwningItemWidgetComponent->Activate();
		OwningItemWidgetComponent->SetVisibility(true);
		OwningItemWidgetComponent->SetWindowVisibility(EWindowVisibility::Visible);
		SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
		OwningItemWidgetComponent->SetWindowVisibility(EWindowVisibility::SelfHitTestInvisible);
		OwningItemWidgetComponent->SetVisibility(false);
		OwningItemWidgetComponent->Deactivate();
	}
	
}

void UNAItemWidget::SwapToggleActionText(FString& ToggleActionStr) const
{
	if (ToggleActionStr == TEXT("Toggle") || ToggleActionStr == TEXT("Close"))
	{
		ToggleActionStr = TEXT("Open");
	}
	else 
	{
		ToggleActionStr = TEXT("Close");
	}
}

void UNAItemWidget::InitItemWidget(UNAItemWidgetComponent* OwningComp, UNAItemData* ItemData)
{
	check(OwningComp != nullptr);
	check(ItemData != nullptr);

	OwningItemWidgetComponent = OwningComp;
	
	if (Item_Name)
	{
		Item_Name->SetText(FText::FromString(ItemData->GetItemName()));
	}
	if (Item_Icon)
	{
		Item_Icon->SetBrushResourceObject(ItemData->GetItemIcon());
	}
	if (Item_Type)
	{
		FString ItemTypeStr = FStringUtils::EnumToDisplayString(ItemData->GetItemType());
		ItemTypeStr.RemoveFromStart("IT_");
		Item_Type->SetText(FText::FromString(ItemTypeStr));
	}
	if (Interaction_Name)
	{
		FNAInteractableData InteractableData;
		if (ItemData->GetInteractableData(InteractableData))
		{
			if (InteractableData.InteractableType == ENAInteractableType::Toggle)
			{
				bIsToggleAction = true;
				ToggleActionText = TEXT("Toggle");
			}
			else
			{
				Interaction_Name->SetText(InteractableData.InteractionName);
			}
		}
	}
}
