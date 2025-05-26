// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Ability/BTTask/BTTaskNode_Spawn.h"
#include "Monster/AI/MonsterAIController.h"
#include "AbilitySystemComponent.h"
#include "Monster/Pawn/MonsterBase.h"
#include "Monster/Ability/GameplayAbility/GA_Spawning.h"

UBTTaskNode_Spawn::UBTTaskNode_Spawn()
{
	NodeName = TEXT("GAS Spawn");
    SpawnAbilityClass = UGA_Spawning::StaticClass();
}

EBTNodeResult::Type UBTTaskNode_Spawn::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn) return EBTNodeResult::Failed;

    AMonsterBase* Monster = Cast<AMonsterBase>(AIPawn);

    UAbilitySystemComponent* AbilitySystemComponent = AIPawn->FindComponentByClass<UAbilitySystemComponent>();
    if (!AbilitySystemComponent) return EBTNodeResult::Failed;

    if (!SpawnAbilityClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_Spawn: SpawnAbilityClass is not set!"));
        return EBTNodeResult::Failed;
    }
    
    // 이게 더 간단할거 같음
    bool bActivated = AbilitySystemComponent->TryActivateAbilityByClass(SpawnAbilityClass);


	return EBTNodeResult::Type();
}
