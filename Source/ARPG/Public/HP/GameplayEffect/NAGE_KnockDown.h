// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "NAGE_KnockDown.generated.h"

class UTargetTagsGameplayEffectComponent;
/**
 * 
 */
UCLASS()
class ARPG_API UNAGE_KnockDown : public UGameplayEffect
{
	GENERATED_BODY()

	UNAGE_KnockDown();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay, meta = (AllowPrivateAccess = "true"))
	UTargetTagsGameplayEffectComponent* TargetTagsGameplayEffectComponent;
};
