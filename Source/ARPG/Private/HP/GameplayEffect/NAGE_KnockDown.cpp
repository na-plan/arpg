// Fill out your copyright notice in the Description page of Project Settings.


#include "HP/GameplayEffect/NAGE_KnockDown.h"

#include "Ability/AttributeSet/NAAttributeSet.h"
#include "GameplayEffectComponents/TargetTagRequirementsGameplayEffectComponent.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UNAGE_KnockDown::UNAGE_KnockDown()
{
	// KnockDown 태그
	TargetTagsGameplayEffectComponent = CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>( TEXT("TargetTagsGameplayEffectComponent") );
	GEComponents.AddUnique( TargetTagsGameplayEffectComponent );
	
	FInheritedTagContainer TagContainer;
	TagContainer.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.KnockDown" ) );
	TargetTagsGameplayEffectComponent->SetAndApplyTargetTagChanges( TagContainer );
	
	// 해제할때까지 지속
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	FGameplayModifierInfo Info;
	Info.Attribute = UNAAttributeSet::GetMovementSpeedAttribute();
	Info.ModifierOp = EGameplayModOp::Override;
	Info.ModifierMagnitude = FScalableFloat( 150.f );

	Modifiers.AddUnique( Info );
}
