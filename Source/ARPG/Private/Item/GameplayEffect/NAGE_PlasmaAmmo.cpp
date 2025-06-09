// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/GameplayEffect/NAGE_PlasmaAmmo.h"

#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UNAGE_PlasmaAmmo::UNAGE_PlasmaAmmo()
{
	StackExpirationPolicy = EGameplayEffectStackingExpirationPolicy::RemoveSingleStackAndRefreshDuration;
	StackDurationRefreshPolicy = EGameplayEffectStackingDurationPolicy::RefreshOnSuccessfulApplication;
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	StackingType = EGameplayEffectStackingType::AggregateByTarget;

	AssetTagsGameplayEffectComponent = CreateDefaultSubobject<UAssetTagsGameplayEffectComponent>( "TargetTagsGameplayEffectComponent" );
	GEComponents.AddUnique( AssetTagsGameplayEffectComponent );

	FInheritedTagContainer TagContainer;
	TagContainer.AddTag( FGameplayTag::RequestGameplayTag( "Weapon.Ammo.Plasma" ) );
	AssetTagsGameplayEffectComponent->SetAndApplyAssetTagChanges( TagContainer );
}
