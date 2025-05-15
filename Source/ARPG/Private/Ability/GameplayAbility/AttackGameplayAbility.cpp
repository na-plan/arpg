// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayAbility/AttackGameplayAbility.h"

void UAttackGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    float AttackRange = 200.0f; // 공격 범위 설정

}

bool UAttackGameplayAbility::IsTargetInRange(AActor* TargetActor, float Range)
{
    if (!TargetActor) return false;

    AActor* OwnerActor = GetAvatarActorFromActorInfo();
    if (!OwnerActor) return false;

    float Distance = FVector::Dist(OwnerActor->GetActorLocation(), TargetActor->GetActorLocation());
    return Distance <= Range;
}
