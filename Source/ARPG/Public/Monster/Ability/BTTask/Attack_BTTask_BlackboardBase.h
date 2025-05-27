// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Attack_BTTask_BlackboardBase.generated.h"

class UGameplayAbility;

/**
 * 
 */
UCLASS()
class ARPG_API UAttack_BTTask_BlackboardBase : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	//EBTNodeResult::Type << 비헤이비어 트리 task succeed, fail, abort, inprogress
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "GAS")
	TSubclassOf<UGameplayAbility> AttackAbilityClass;
	
};
