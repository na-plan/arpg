// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/GameplayEffect/NAGE_ConsumeAmmo.h"

#include "Weapon/AttributeSet/NAWeaponAttributeSet.h"

UNAGE_ConsumeAmmo::UNAGE_ConsumeAmmo()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	
	FGameplayModifierInfo AmmoModifier;
	AmmoModifier.Attribute = UNAWeaponAttributeSet::GetTotalAmmoCountAttribute();
	AmmoModifier.ModifierOp = EGameplayModOp::Additive;
	AmmoModifier.ModifierMagnitude = FScalableFloat(1.f); // todo: Int!

	Modifiers.Add(AmmoModifier);
}
