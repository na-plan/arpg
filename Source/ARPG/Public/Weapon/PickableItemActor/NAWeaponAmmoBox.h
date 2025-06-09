// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemActor/NAPickableItemActor.h"
#include "NAWeaponAmmoBox.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;

UCLASS()
class ARPG_API ANAWeaponAmmoBox : public ANAPickableItemActor
{
	GENERATED_BODY()

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Replicated, Category="Ability", meta=(AllowPrivateAccess="true"))
	TSubclassOf<UGameplayEffect> AmmoEffectType;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Replicated, Category="Combat", meta=(AllowPrivateAccess="true") )
	int32 AmmoCount = 1;
	
public:
	// Sets default values for this actor's properties
	ANAWeaponAmmoBox();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual bool ExecuteInteract_Implementation(AActor* InteractorActor) override;

	virtual void EndInteract_Implementation(AActor* InteractorActor) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	// Called every frame
	virtual void Tick( float DeltaTime ) override;

	TSubclassOf<UGameplayEffect> GetAmmoEffectType() const { return AmmoEffectType; }
};
