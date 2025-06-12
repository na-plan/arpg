// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "NAAmmoWidget.generated.h"

class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class ARPG_API UNAAmmoWidget : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true", BindWidget))
	UTextBlock* AmmoCount;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true", BindWidget))
	UImage* BackgroundHologram;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true", BindWidget))
	UImage* FarHologramFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true", BindWidget))
	UImage* NearHologramFrame;

public:
	UTextBlock* GetAmmoCountText() const { return AmmoCount; }

	void SetHologramFrame( UMaterialInstanceDynamic* Far, UMaterialInstanceDynamic* Near ) const;

	void SetBackgroundHologram( UMaterialInstanceDynamic* InBackgroundHologram ) const;
};
