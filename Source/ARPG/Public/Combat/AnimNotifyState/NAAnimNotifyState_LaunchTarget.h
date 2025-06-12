// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "GameplayEffectTypes.h"

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "NAAnimNotifyState_LaunchTarget.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNAAnimNotifyState_LaunchTarget : public UAnimNotifyState
{
	GENERATED_BODY()


	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	FName SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap", meta = (AllowPrivateAccess = "true"))
	float SphereRadius = 20.f;

	float BaseDamage = 0;

	UPROPERTY()
	TSet<AActor*> AppliedActors;

	FGameplayEffectContextHandle ContextHandle;

	FGameplayEffectSpecHandle SpecHandle;
	
	
};
