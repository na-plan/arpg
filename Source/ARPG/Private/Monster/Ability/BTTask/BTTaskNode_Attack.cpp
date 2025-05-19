// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Ability/BTTask/BTTaskNode_Attack.h"
#include "Monster/AI/MonsterAIController.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"


UBTTaskNode_Attack::UBTTaskNode_Attack()
{
	NodeName = TEXT("GAS Attack");
}

EBTNodeResult::Type UBTTaskNode_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn) return EBTNodeResult::Failed;

    UAbilitySystemComponent* AbilitySystemComponent = AIPawn->FindComponentByClass<UAbilitySystemComponent>();
    if (!AbilitySystemComponent) return EBTNodeResult::Failed;

    if (!AttackAbilityClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_Attack: AttackAbilityClass is not set!"));
        return EBTNodeResult::Failed;
    }

    //여기서 오류가 failed 로 넘어가짐     AttackAbilityClass는 잘 찾아오는데 AttackAbilityClass는 실행이 안됌
    if (AbilitySystemComponent->TryActivateAbilityByClass(AttackAbilityClass))
    {
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}
