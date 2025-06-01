// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NAPickableItemActor.h"
#include "GameFramework/Actor.h"
#include "NAWeapon.generated.h"

class UNAMontageCombatComponent;

struct FCombatUpdatePredication : FItemPatchHelper::FDefaultUpdatePredication<UNAMontageCombatComponent>
{
	virtual void operator()( AActor* InOuter, UNAMontageCombatComponent* InComponent,
		UNAMontageCombatComponent* InOldComponent, const FNAItemBaseTableRow* InRow,
		const EItemMetaDirtyFlags DirtyFlags ) const override;
};

struct FCombatInstanceUpdatePredication : FCombatUpdatePredication
{
	virtual void operator()( AActor* InOuter, UNAMontageCombatComponent* InComponent,
		UNAMontageCombatComponent* InOldComponent, const FNAItemBaseTableRow* InRow,
		const EItemMetaDirtyFlags DirtyFlags ) const override;
};

UCLASS()
class ARPG_API ANAWeapon : public ANAPickableItemActor
{
	GENERATED_BODY()

	friend class UNAItemEngineSubsystem;
	friend struct FCombatUpdatePredication;

	UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess="true"))
	UNAMontageCombatComponent* CombatComponent;
	
public:
	// Sets default values for this actor's properties
	ANAWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#if WITH_EDITOR
    virtual void ExecuteItemPatch(UClass* ClassToPatch, const FNAItemBaseTableRow* PatchData, EItemMetaDirtyFlags PatchFlags) override;
#endif

public:
	// Called every frame
	virtual void Tick( float DeltaTime ) override;
};
