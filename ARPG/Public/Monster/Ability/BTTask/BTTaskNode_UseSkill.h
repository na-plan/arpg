// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskNode_UseSkill.generated.h"


USTRUCT()
struct ARPG_API FNAMonsterOwnSkillTableRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	//각 몬스터별 보유하고 있는 스킬 목록


};

/**
 * 
 */
UCLASS()
class ARPG_API UBTTaskNode_UseSkill : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTaskNode_UseSkill();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	/* 호출할 어빌리티 클래스 */
	UPROPERTY(EditAnywhere, Category = "GAS")
	TSubclassOf<class UGameplayAbility> SkillAbilityClass;
	
	
};
