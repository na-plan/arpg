// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"

#include "CoreMinimal.h"
#include "AIController.h"
#include "MonsterAIController.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API AMonsterAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

protected:
	/*Spawn 위치 기준 일정 범위 나갔는지 확인하는 기능*/
	void CheckSpawnRadius();

	//Ability system 완성후에 만들어야 할거 같음
	//UFUNCTION()
	//void OnDamaged(float CurrentHP, float MaxHP);

	
};
