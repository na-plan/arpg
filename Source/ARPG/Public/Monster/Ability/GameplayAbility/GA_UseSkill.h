// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_UseSkill.generated.h"


USTRUCT()
struct ARPG_API FNAMonsterSkillDataTableRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	//사거리(원거리 근거리)
	UPROPERTY(EditAnywhere, Category = "Skill")
	float Distance;
	UPROPERTY(EditAnywhere, Category = "Skill")
	float Damage;
	UPROPERTY(EditAnywhere, Category = "Skill")
	float SkillCoolTime;
	// 여러개의 몽타주 사용할 경우 차례대로 실행
	UPROPERTY(EditAnywhere, Category = "SkillAnimation")
	TArray<UAnimMontage*> SkillAnimation;



};



/**
 * 
 */
UCLASS()
class ARPG_API UGA_UseSkill : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	//UPROPERTY(EditDefaultsOnly)
	//FSkillDataRow* SkillData;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);


	
};
