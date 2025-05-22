// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NACombatComponent.h"
#include "NAMontageCombatComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_API UNAMontageCombatComponent : public UNACombatComponent
{
	GENERATED_BODY()

	// 공격 시 수행할 Sound
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	USoundBase* AttackSound = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	UAnimMontage* CachedAttackMontage = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	float CachedAttackMontagePlayRate = 1.f;

public:
	// Sets default values for this component's properties
	UNAMontageCombatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual bool IsAbleToAttack() override;

	virtual void OnAttack_Implementation() override;

	UFUNCTION(NetMulticast, Reliable)
	void Multi_PlaySound() const;

	virtual float GetNextAttackTime() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
