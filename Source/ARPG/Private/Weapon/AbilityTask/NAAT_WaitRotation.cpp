// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/AbilityTask/NAAT_WaitRotation.h"

#include "NACharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UNAAT_WaitRotation::UNAAT_WaitRotation()
{
	bTickingTask = true;
}

void UNAAT_WaitRotation::Activate()
{
	Super::Activate();

	if ( const ANACharacter* Character = Cast<ANACharacter>( GetAvatarActor() ) )
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	}

	Rotate();
}

void UNAAT_WaitRotation::ForceRotate() const
{
	if ( ANACharacter* Character = Cast<ANACharacter>( GetAvatarActor() ) )
	{
		FVector CurrentForward = Character->GetActorForwardVector();
		const float Delta = TargetForward.Dot( CurrentForward );
		
		if ( !FMath::IsNearlyEqual( Delta, 1, 0.1f ) )
		{
			Character->SetStopOverrideControlRotation( true, TargetRotator );
			Character->SetStopOverrideControlRotation( false, {} );
		}
	}
}

void UNAAT_WaitRotation::OnDestroy( bool bInOwnerFinished )
{
	Super::OnDestroy( bInOwnerFinished );
	
	ForceRotate();

	if ( const ANACharacter* Character = Cast<ANACharacter>( GetAvatarActor() ) )
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

void UNAAT_WaitRotation::Rotate()
{
	if ( ANACharacter* Character = Cast<ANACharacter>( GetAvatarActor() ) )
	{
		FVector CurrentForward = Character->GetActorForwardVector();
		const FQuat NewRotation = FMath::QInterpTo( CurrentForward.ToOrientationQuat(), TargetRotation, UGameplayStatics::GetWorldDeltaSeconds( GetWorld() ), 20.f );
		Character->SetStopOverrideControlRotation( true, NewRotation.Rotator() );
		const float Delta = TargetForward.Dot( NewRotation.GetForwardVector() );
		
		if ( FMath::IsNearlyEqual( Delta, 1, 0.1f ) )
		{
			// 서버일 경우 받아서 다음 처리 수행, 클라이언트일 경우 회전만 수행하고 종료
			OnRotationCompleted.ExecuteIfBound();
			UE_LOG( LogTemp, Display, TEXT("Rotation finished at %s in the net mode %d"), *CurrentForward.ToString(), GetWorld()->GetNetMode() );
			Character->SetStopOverrideControlRotation( false, {} );
			Character->GetCharacterMovement()->bOrientRotationToMovement = true;
			EndTask();
		}
	}
}

void UNAAT_WaitRotation::TickTask( float DeltaTime )
{
	Super::TickTask( DeltaTime );
	UE_LOG(LogTemp, Display, TEXT("Rotation from %d %s"), GetWorld()->GetNetMode(), *GetAvatarActor()->GetActorRotation().ToString() );
	Rotate();
}

UNAAT_WaitRotation* UNAAT_WaitRotation::WaitRotation( UGameplayAbility* OwningAbility, FName TaskInstanceName,
	const FQuat& TargetRotation )
{
	const auto RetVal = NewAbilityTask<UNAAT_WaitRotation>( OwningAbility, TaskInstanceName );
	RetVal->TargetRotation = TargetRotation;
	RetVal->TargetRotator = TargetRotation.Rotator();
	RetVal->TargetForward = TargetRotation.Vector();
	RetVal->Ability = OwningAbility;
	return RetVal;
}
