// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "NAGE_Dead.generated.h"

class UTargetTagsGameplayEffectComponent;
/**
 * 
 */
UCLASS()
class ARPG_API UNAGE_Dead : public UGameplayEffect
{
	GENERATED_BODY()

	UNAGE_Dead();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess=true))
	UTargetTagsGameplayEffectComponent* TargetTagsGameplayEffectComponent;
};
