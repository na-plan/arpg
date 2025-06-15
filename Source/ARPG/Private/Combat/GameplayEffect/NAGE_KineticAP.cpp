// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/GameplayEffect/NAGE_KineticAP.h"

#include "Combat/AttributeSet/NAKineticAttributeSet.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"

UNAGE_KineticAP::UNAGE_KineticAP()
{
	AssetTagsComponent = CreateDefaultSubobject<UAssetTagsGameplayEffectComponent>("TargetTagsGameplayEffectComponent");
	GEComponents.AddUnique( AssetTagsComponent );

	FInheritedTagContainer Container;
	Container.AddTag( FGameplayTag::RequestGameplayTag( TEXT( "Data.Kinetic") ) );
	AssetTagsComponent->SetAndApplyAssetTagChanges( Container );

	FGameplayModifierInfo Modifier;
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.Attribute = UNAKineticAttributeSet::GetAPAttribute();
	
	FSetByCallerFloat CallerValue;
	CallerValue.DataTag = FGameplayTag::RequestGameplayTag( TEXT( "Data.KineticAP" ) );
	Modifier.ModifierMagnitude = CallerValue;

	Modifiers.Add( Modifier );
}
