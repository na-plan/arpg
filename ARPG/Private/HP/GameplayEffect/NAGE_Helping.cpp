// Fill out your copyright notice in the Description page of Project Settings.


#include "HP/GameplayEffect/NAGE_Helping.h"

#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UNAGE_Helping::UNAGE_Helping()
{
	// Helping 태그
	TargetTagsGameplayEffectComponent = CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>( "TargetTagsGameplayEffectComponent" );
	GEComponents.AddUnique( TargetTagsGameplayEffectComponent );

	FInheritedTagContainer TagContainer;
	TagContainer.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.Helping" ) );
	TargetTagsGameplayEffectComponent->SetAndApplyTargetTagChanges( TagContainer );

	DurationPolicy = EGameplayEffectDurationType::Infinite;
}
