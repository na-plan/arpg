// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AttributeSet/MonsterAttributeSet.h"

#include "Net/UnrealNetwork.h"

void UMonsterAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UMonsterAttributeSet, Health);
	DOREPLIFETIME(UMonsterAttributeSet, Power);


}
