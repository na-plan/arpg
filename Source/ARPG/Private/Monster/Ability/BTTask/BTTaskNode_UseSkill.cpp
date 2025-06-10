// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Ability/BTTask/BTTaskNode_UseSkill.h"
#include "Monster/Ability/GameplayAbility/GA_UseSkill.h"
#include "Monster/AI/MonsterAIController.h"
#include "AbilitySystemComponent.h"
#include "Monster/Pawn/MonsterBase.h"


UBTTaskNode_UseSkill::UBTTaskNode_UseSkill()
{
	NodeName = TEXT("GAS UseSKill");
	SkillAbilityClass = UGA_UseSkill::StaticClass();
}

EBTNodeResult::Type UBTTaskNode_UseSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn) return EBTNodeResult::Failed;

    AMonsterBase* Monster = Cast<AMonsterBase>(AIPawn);

    UAbilitySystemComponent* AbilitySystemComponent = AIPawn->FindComponentByClass<UAbilitySystemComponent>();
    if (!AbilitySystemComponent) return EBTNodeResult::Failed;

    if (!SkillAbilityClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_UseSkill: SkillAbilityClass is not set!"));
        return EBTNodeResult::Failed;
    }

    // 이게 더 간단할거 같음
    bool bActivated = AbilitySystemComponent->TryActivateAbilityByClass(SkillAbilityClass);




	return EBTNodeResult::Type();
}
