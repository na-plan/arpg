// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/GameplayEffect/NAGE_UseActivePoint.h"

#include "Ability/AttributeSet/NAAttributeSet.h"

UNAGE_UseActivePoint::UNAGE_UseActivePoint()
{
	// 체력 감소 효과는 계속해서 지속된다
	// 만약 최대 체력 감소 효과를 부여하려면, Instant를 사용하면 된다.
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	
	FGameplayModifierInfo APModifier;
	APModifier.Attribute = UNAAttributeSet::GetAPAttribute();
	APModifier.ModifierOp = EGameplayModOp::Additive;
	APModifier.ModifierMagnitude = FScalableFloat(-10.f);

	DurationMagnitude = FScalableFloat(2.f);
	Modifiers.Add(APModifier);
}
