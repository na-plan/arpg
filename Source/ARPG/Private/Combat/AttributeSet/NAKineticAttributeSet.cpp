// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/AttributeSet/NAKineticAttributeSet.h"

void UNAKineticAttributeSet::PostGameplayEffectExecute( const FGameplayEffectModCallbackData& Data )
{
	Super::PostGameplayEffectExecute( Data );

	if ( GetAP() <= 0.f )
	{
		SetAP( 0.f );
	}

	if ( GetAP() > GetMaxAP() )
	{
		SetAP( GetMaxAP() );
	}
}
