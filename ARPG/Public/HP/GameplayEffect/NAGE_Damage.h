// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"
#include "UObject/Object.h"
#include "NAGE_Damage.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNAGE_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UNAGE_Damage();

	UPROPERTY()
	UAssetTagsGameplayEffectComponent* AssetTagsComponent;
};
