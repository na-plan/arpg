// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/AttributeSet/NAKineticAttributeSet.h"

#include "Net/UnrealNetwork.h"

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

void UNAKineticAttributeSet::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
	DOREPLIFETIME( UNAKineticAttributeSet, AP );
}
