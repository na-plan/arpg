// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/AbilityTask/NAGA_WaitRotation.h"

UNAAT_WaitRotation::UNAAT_WaitRotation()
{
	bTickingTask = true;
}

void UNAAT_WaitRotation::Activate()
{
	Super::Activate();
}

void UNAAT_WaitRotation::TickTask( float DeltaTime )
{
	Super::TickTask( DeltaTime );

	if ( APawn* Pawn = Cast<APawn>( GetAvatarActor() ) )
	{
		const FQuat NewRotation = FMath::QInterpTo( Pawn->GetActorQuat(), TargetRotation, DeltaTime, 20.f );
		Pawn->SetActorRotation( NewRotation );

		if ( TargetRotation.Equals( NewRotation ) )
		{
			// 실행했다면 반드시 Bound 했어야 했기 떄문에 예외처리 안함
			OnRotationCompleted.Execute();
			EndTask();
		}
	}
}

UNAAT_WaitRotation* UNAAT_WaitRotation::WaitRotation( UGameplayAbility* OwningAbility, FName TaskInstanceName,
	const FQuat& TargetRotation )
{
	const auto RetVal = NewAbilityTask<UNAAT_WaitRotation>( OwningAbility, TaskInstanceName );
	RetVal->TargetRotation = TargetRotation;
	RetVal->Ability = OwningAbility;
	return RetVal;
}
