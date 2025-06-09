// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "NAPickableItemActor.h"
#include "GameFramework/Actor.h"
#include "NAWeapon.generated.h"

class UGameplayEffect;
class UNiagaraComponent;
class UNAMontageCombatComponent;

UCLASS()
class ARPG_API ANAWeapon : public ANAPickableItemActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

	friend class UNAItemEngineSubsystem;
	friend struct FCombatUpdatePredication;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Replicated, Category="Combat", meta=(AllowPrivateAccess="true"))
	UNAMontageCombatComponent* CombatComponent;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Replicated, Category="Combat", meta=(AllowPrivateAccess="true"))
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category="FX", meta=(AllowPrivateAccess="true"))
	UNiagaraComponent* MuzzleFlashComponent;
	
public:
	// Sets default values for this actor's properties
	ANAWeapon();

	FORCEINLINE virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Called every frame
	virtual void Tick( float DeltaTime ) override;
};
