// Fill out your copyright notice in the Description page of Project Settings.


#include "HP/GameplayEffect/NAGE_Heal.h"

#include "Ability/AttributeSet/NAAttributeSet.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"

UNAGE_Heal::UNAGE_Heal()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	
	FGameplayModifierInfo DamageModifier;
	DamageModifier.Attribute = UNAAttributeSet::GetHealthAttribute();
	DamageModifier.ModifierOp = EGameplayModOp::Additive;
	
	AssetTagsComponent = CreateDefaultSubobject<UAssetTagsGameplayEffectComponent>("TargetTagsGameplayEffectComponent");
	GEComponents.AddUnique( AssetTagsComponent );

	FInheritedTagContainer Container;
	Container.AddTag( FGameplayTag::RequestGameplayTag( TEXT( "Data.Health") ) );
	AssetTagsComponent->SetAndApplyAssetTagChanges( Container );
	
	FSetByCallerFloat CallerValue;
	CallerValue.DataTag = FGameplayTag::RequestGameplayTag( TEXT( "Data.Heal" ) );
	DamageModifier.ModifierMagnitude = CallerValue;
	Modifiers.Add(DamageModifier);
}
