// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AttributeSet/SkillAttributeSet.h"
#include "Net/UnrealNetwork.h"

void USkillAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USkillAttributeSet, SkillCooldownTime);
	DOREPLIFETIME(USkillAttributeSet, SkillDamage);
	DOREPLIFETIME(USkillAttributeSet, SkillCost);


}
