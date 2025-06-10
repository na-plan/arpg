// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AttributeSet/SkillAttributeSet.h"
#include "Net/UnrealNetwork.h"

void USkillAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USkillAttributeSet, CoolTime);
	DOREPLIFETIME(USkillAttributeSet, Damage);
	DOREPLIFETIME(USkillAttributeSet, Cost);


}
