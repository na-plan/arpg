// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Ability/BTTask/Attack_BTTask_BlackboardBase.h"
#include "Monster/AI/MonsterAIController.h"
#include "Monster/Pawn/MonsterBase.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Ability/GameplayAbility/AttackGameplayAbility.h"


EBTNodeResult::Type UAttack_BTTask_BlackboardBase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AMonsterAIController* AIController = Cast<AMonsterAIController>(OwnerComp.GetAIOwner()))
	{
        if (AMonsterBase* Monster = Cast<AMonsterBase>(AIController->GetPawn()))
        {
            UAbilitySystemComponent* AbilitySystemComponent = Monster->GetAbilitySystemComponent();
            // 공격 완성 시켜야됌
            FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(UAttackGameplayAbility::StaticClass());

            if (AbilitySpec && AbilitySystemComponent->TryActivateAbility(AbilitySpec->Handle))
            {
                UE_LOG(LogTemp, Warning, TEXT("Monster Ability Activated!"));
                return EBTNodeResult::Succeeded;
            }
        }
	}
    return EBTNodeResult::Failed;
}
