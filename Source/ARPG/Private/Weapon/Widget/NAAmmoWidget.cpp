// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Widget/NAAmmoWidget.h"

#include "Components/Image.h"


void UNAAmmoWidget::SetHologramFrame( UMaterialInstanceDynamic* Far, UMaterialInstanceDynamic* Near ) const
{
	FarHologramFrame->SetBrushFromMaterial( Far );
	NearHologramFrame->SetBrushFromMaterial( Near );
}

void UNAAmmoWidget::SetBackgroundHologram( UMaterialInstanceDynamic* InBackgroundHologram ) const
{
	BackgroundHologram->SetBrushFromMaterial( InBackgroundHologram );
}
