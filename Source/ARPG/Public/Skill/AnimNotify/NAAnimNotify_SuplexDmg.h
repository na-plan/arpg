// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "NAAnimNotify_SuplexDmg.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNAAnimNotify_SuplexDmg : public UAnimNotify
{
	GENERATED_BODY()
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	FGameplayEffectContextHandle ContextHandle;

	FGameplayEffectSpecHandle SpecHandle;
	
	
};
