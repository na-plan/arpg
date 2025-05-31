// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NAReviveWidget.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class ARPG_API UNAReviveWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetImage( UMaterialInstanceDynamic* Instance ) const;

protected:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Progress", meta=( BindWidget ) )
	UImage* ProgressImage; 
};
