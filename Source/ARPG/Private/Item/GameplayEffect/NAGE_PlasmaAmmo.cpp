// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/GameplayEffect/NAGE_PlasmaAmmo.h"

#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UNAGE_PlasmaAmmo::UNAGE_PlasmaAmmo()
{
	StackExpirationPolicy = EGameplayEffectStackingExpirationPolicy::RemoveSingleStackAndRefreshDuration;
	StackDurationRefreshPolicy = EGameplayEffectStackingDurationPolicy::NeverRefresh;
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	StackingType = EGameplayEffectStackingType::AggregateByTarget;
	
	TargetTagsGameplayEffectComponent = CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>( "TargetTagsGameplayEffectComponent" );
	GEComponents.AddUnique( TargetTagsGameplayEffectComponent );

	FInheritedTagContainer TagContainer;
	TagContainer.AddTag( FGameplayTag::RequestGameplayTag( "Weapon.Ammo.Plasma" ) );
	TargetTagsGameplayEffectComponent->SetAndApplyTargetTagChanges( TagContainer );
}
