// Fill out your copyright notice in the Description page of Project Settings.


#include "HP/GameplayEffect/NAGE_Damage.h"

#include "Ability/AttributeSet/NAAttributeSet.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"

UNAGE_Damage::UNAGE_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	
	FGameplayEffectExecutionDefinition f;
	f.CalculationClass;
	Executions;
	FGameplayModifierInfo DamageModifier;
	DamageModifier.Attribute = UNAAttributeSet::GetHealthAttribute();
	DamageModifier.ModifierOp = EGameplayModOp::Additive;

	AssetTagsComponent = CreateDefaultSubobject<UAssetTagsGameplayEffectComponent>("TargetTagsGameplayEffectComponent");
	GEComponents.AddUnique( AssetTagsComponent );

	FInheritedTagContainer Container;
	Container.AddTag( FGameplayTag::RequestGameplayTag( TEXT( "Data.Health") ) );
	AssetTagsComponent->SetAndApplyAssetTagChanges( Container );
	
	FSetByCallerFloat CallerValue;
	CallerValue.DataTag = FGameplayTag::RequestGameplayTag( TEXT( "Data.Damage" ) );
	DamageModifier.ModifierMagnitude = CallerValue;
	Modifiers.Add(DamageModifier);
}
