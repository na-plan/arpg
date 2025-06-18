// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/AbilityTask/NAAT_ConsumeKineticGrabAP.h"

#include "AbilitySystemComponent.h"
#include "Combat/AttributeSet/NAKineticAttributeSet.h"
#include "Combat/GameplayEffect/NAGE_KineticAP.h"

UNAAT_ConsumeKineticGrabAP::UNAAT_ConsumeKineticGrabAP()
{
	bTickingTask = true;
}

UNAAT_ConsumeKineticGrabAP* UNAAT_ConsumeKineticGrabAP::WaitAPDepleted( UGameplayAbility* OwningAbility,
                                                                        const FName TaskInstanceName )
{
	auto* ReturnValue = NewAbilityTask<UNAAT_ConsumeKineticGrabAP>( OwningAbility, TaskInstanceName );
	return ReturnValue;
}

void UNAAT_ConsumeKineticGrabAP::TickTask( float DeltaTime )
{
	Super::TickTask( DeltaTime );

	const UNAKineticAttributeSet* AttributeSet = Cast<UNAKineticAttributeSet>( AbilitySystemComponent->GetAttributeSet( UNAKineticAttributeSet::StaticClass() ) );
	const float Decrease = -(10.f * DeltaTime);
	const float AP = AttributeSet->GetAP() + Decrease;
	
	if ( AP <= 0.f )
	{
		OnAPDepleted.Broadcast();
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("%hs: AP decreases to %f"), __FUNCTION__, AP)

	const FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec( UNAGE_KineticAP::StaticClass(), 1.f, Context );
	SpecHandle.Data->SetSetByCallerMagnitude( FGameplayTag::RequestGameplayTag( "Data.KineticAP" ),  Decrease);
	const FActiveGameplayEffectHandle Handle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf( *SpecHandle.Data.Get() );
	check( Handle.WasSuccessfullyApplied() );
}
