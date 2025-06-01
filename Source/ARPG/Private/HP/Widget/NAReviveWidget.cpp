// Fill out your copyright notice in the Description page of Project Settings.


#include "HP/Widget/NAReviveWidget.h"

#include "Components/Image.h"

void UNAReviveWidget::SetImage( UMaterialInstanceDynamic* Instance ) const
{
	ProgressImage->SetBrushFromMaterial( Instance );
}
