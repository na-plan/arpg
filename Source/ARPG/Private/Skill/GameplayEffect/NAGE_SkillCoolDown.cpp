// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/GameplayEffect/NAGE_SkillCoolDown.h"

#include "AbilitySystemLog.h"
#include "Ability/AttributeSet/NAAttributeSet.h"


UNAGE_SkillCoolDown::UNAGE_SkillCoolDown()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FGameplayModifierInfo APModifier;
	APModifier.Attribute = UNAAttributeSet::GetAPAttribute();
	APModifier.ModifierOp = EGameplayModOp::Additive;
	APModifier.ModifierMagnitude = FScalableFloat(-10.f);

	DurationMagnitude = FScalableFloat(5.f);
	Modifiers.Add(APModifier);

}
