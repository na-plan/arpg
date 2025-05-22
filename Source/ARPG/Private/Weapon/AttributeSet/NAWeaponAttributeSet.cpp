// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/AttributeSet/NAWeaponAttributeSet.h"

#include "Net/UnrealNetwork.h"

void UNAWeaponAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UNAWeaponAttributeSet, LoadedAmmoCount);
	DOREPLIFETIME(UNAWeaponAttributeSet, TotalAmmoCount);
}
