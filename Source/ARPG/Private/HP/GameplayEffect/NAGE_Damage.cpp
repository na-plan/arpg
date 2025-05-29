// Fill out your copyright notice in the Description page of Project Settings.


#include "HP/GameplayEffect/NAGE_Damage.h"

#include "AbilitySystemLog.h"
#include "Ability/AttributeSet/NAAttributeSet.h"

UNAGE_Damage::UNAGE_Damage()
{
	// 체력 감소 효과는 계속해서 지속된다
	// 만약 최대 체력 감소 효과를 부여하려면, Instant를 사용하면 된다.
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	
	FGameplayModifierInfo DamageModifier;
	DamageModifier.Attribute = UNAAttributeSet::GetHealthAttribute();
	DamageModifier.ModifierOp = EGameplayModOp::Additive;
	DamageModifier.ModifierMagnitude = FScalableFloat(-10.f);
	Modifiers.Add(DamageModifier);
}
