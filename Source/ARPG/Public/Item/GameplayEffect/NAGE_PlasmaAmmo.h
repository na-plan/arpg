// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "NAGE_PlasmaAmmo.generated.h"

class UAssetTagsGameplayEffectComponent;
/**
 * 
 */
UCLASS()
class ARPG_API UNAGE_PlasmaAmmo : public UGameplayEffect
{
	GENERATED_BODY()

	UNAGE_PlasmaAmmo();

	UPROPERTY()
	UAssetTagsGameplayEffectComponent* AssetTagsGameplayEffectComponent;
};
