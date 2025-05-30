// Fill out your copyright notice in the Description page of Project Settings.


#include "HP/GameplayEffect/NAGE_Dead.h"

#include "Ability/AttributeSet/NAAttributeSet.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UNAGE_Dead::UNAGE_Dead()
{
	// Dead 태그
	TargetTagsGameplayEffectComponent = CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>("TargetTagsGameplayEffectComponent");
	GEComponents.AddUnique( TargetTagsGameplayEffectComponent );
	
	FInheritedTagContainer TagContainer;
	TagContainer.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.Dead" ) );
	TagContainer.Removed.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.KnockDown" ) );
	TargetTagsGameplayEffectComponent->SetAndApplyTargetTagChanges( TagContainer );

	DurationPolicy = EGameplayEffectDurationType::Infinite;

	// 이동속력 감소
	FGameplayModifierInfo Info;
	Info.Attribute = UNAAttributeSet::GetMovementSpeedAttribute();
	Info.ModifierOp = EGameplayModOp::Override;
	Info.ModifierMagnitude = FScalableFloat( 0.f );

	Modifiers.AddUnique( Info );
}
