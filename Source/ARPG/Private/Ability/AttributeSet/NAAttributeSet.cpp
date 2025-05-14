// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AttributeSet/NAAttributeSet.h"

#include "Net/UnrealNetwork.h"

void UNAAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UNAAttributeSet, Health);
	DOREPLIFETIME(UNAAttributeSet, AP);
}
