// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "NAGE_Helping.generated.h"

class UTargetTagsGameplayEffectComponent;
/**
 * 
 */
UCLASS()
class ARPG_API UNAGE_Helping : public UGameplayEffect
{
	GENERATED_BODY()

	UNAGE_Helping();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay, meta = (AllowPrivateAccess = "true"))
	UTargetTagsGameplayEffectComponent* TargetTagsGameplayEffectComponent;
};
