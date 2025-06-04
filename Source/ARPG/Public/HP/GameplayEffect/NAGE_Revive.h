// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "NAGE_Revive.generated.h"

class UTargetTagRequirementsGameplayEffectComponent;
class UTargetTagsGameplayEffectComponent;
/**
 * 
 */
UCLASS()
class ARPG_API UNAGE_Revive : public UGameplayEffect
{
	GENERATED_BODY()

	UNAGE_Revive();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay, meta = (AllowPrivateAccess = "true"))
	UTargetTagsGameplayEffectComponent* TargetTagsGameplayEffectComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay, meta = (AllowPrivateAccess = "true"))
	UTargetTagRequirementsGameplayEffectComponent* TargetTagRequirementsGameplayEffectComponent;
};
