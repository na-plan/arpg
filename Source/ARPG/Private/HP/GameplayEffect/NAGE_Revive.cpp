// Fill out your copyright notice in the Description page of Project Settings.


#include "HP/GameplayEffect/NAGE_Revive.h"

#include "GameplayEffectComponents/TargetTagRequirementsGameplayEffectComponent.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UNAGE_Revive::UNAGE_Revive()
{
	// Reviving 태그
	TargetTagsGameplayEffectComponent = CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>( "TargetTagsGameplayEffectComponent" );
	GEComponents.AddUnique( TargetTagsGameplayEffectComponent );

	FInheritedTagContainer TagContainer;
	TagContainer.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.Reviving" ) );
	TargetTagsGameplayEffectComponent->SetAndApplyTargetTagChanges( TagContainer );

	// 부활하는 동안에는 기절 태그가 필요함
	TargetTagRequirementsGameplayEffectComponent = CreateDefaultSubobject<UTargetTagRequirementsGameplayEffectComponent>( "TargetTagRequirementsGameplayEffectComponent" );
	GEComponents.AddUnique( TargetTagRequirementsGameplayEffectComponent );
	
	TargetTagRequirementsGameplayEffectComponent->ApplicationTagRequirements.RequireTags.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.KnockDown" ) );

	DurationPolicy = EGameplayEffectDurationType::Infinite;
}
