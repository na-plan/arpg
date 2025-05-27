// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NACombatComponent.h"
#include "NAMontageCombatComponent.generated.h"

/*
 * 몽타주를 기준으로 사용 시간을 계산하는 전투 컴포넌트
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_API UNAMontageCombatComponent : public UNACombatComponent
{
	GENERATED_BODY()

	// 공격 시 수행할 Sound
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	USoundBase* AttackSound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	UAnimMontage* AttackMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	float MontagePlayRate = 1.f;

public:
	// Sets default values for this component's properties
	UNAMontageCombatComponent();

	void SetAttackMontage( UAnimMontage* InAttackMontage ) { AttackMontage = InAttackMontage; }

	float GetMontagePlayRate() const { return MontagePlayRate; }

	virtual bool IsAbleToAttack() override;
	
	UAnimMontage* GetMontage() const { return AttackMontage; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	virtual void OnAttack_Implementation() override;

	UFUNCTION(NetMulticast, Reliable)
	void Multi_PlaySound() const;

	virtual float GetNextAttackTime() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
