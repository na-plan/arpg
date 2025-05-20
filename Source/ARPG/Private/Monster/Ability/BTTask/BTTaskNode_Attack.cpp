// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Ability/BTTask/BTTaskNode_Attack.h"
#include "Monster/AI/MonsterAIController.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "Monster/Pawn/MonsterBase.h"
#include "Monster/Ability/GameplayAbility/GA_MonsterAttack.h"

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

    AMonsterBase* Monster = Cast<AMonsterBase>(AIPawn);

    //Monster 에서 하지 않고 FindComponentByClass로 해서 찾아와도 될거 같음
    UAbilitySystemComponent* AbilitySystemComponent = AIPawn->FindComponentByClass<UAbilitySystemComponent>();

    if (!AbilitySystemComponent) return EBTNodeResult::Failed;

    if (!AttackAbilityClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_Attack: AttackAbilityClass is not set!"));
        return EBTNodeResult::Failed;
    }

    //여기서 오류가 failed 로 넘어가짐     AttackAbilityClass는 잘 찾아오는데 AttackAbilityClass는 실행이 안됌
    // 실제 사용할 어빌리티 클래스
    TSubclassOf<UGameplayAbility> AbilityToActivate = UGA_MonsterAttack::StaticClass();
    bool bActivated = AbilitySystemComponent->TryActivateAbilityByClass(AbilityToActivate);

    return bActivated ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
