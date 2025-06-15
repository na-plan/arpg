// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "NAGE_KineticAP.generated.h"

class UAssetTagsGameplayEffectComponent;
/**
 * 
 */
UCLASS()
class ARPG_API UNAGE_KineticAP : public UGameplayEffect
{
	GENERATED_BODY()

	UNAGE_KineticAP();

	UPROPERTY()
	UAssetTagsGameplayEffectComponent* AssetTagsComponent;
};
