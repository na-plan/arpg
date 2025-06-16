// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/AbilityTask/NAAT_MoveActorTo.h"

#include "Combat/GameplayAbility/NAGA_KineticGrab.h"
#include "ARPG/Public/Combat/PhysicsHandleComponent/NAKineticComponent.h"

UNAAT_MoveActorTo* UNAAT_MoveActorTo::MoveActorTo( UGameplayAbility* OwningAbility, FName TaskInstanceName,
                                                   AActor* Origin, AActor* Target, UPrimitiveComponent* InTargetBoundComponent )
{
	auto* ReturnValue = NewAbilityTask<UNAAT_MoveActorTo>( OwningAbility, TaskInstanceName );
	ReturnValue->OriginActor = Origin;
	ReturnValue->TargetActor = Target;
	ReturnValue->TargetBoundComponent = InTargetBoundComponent;
	return ReturnValue;
}

void UNAAT_MoveActorTo::Activate()
{
	Super::Activate();

	if ( TargetBoundComponent.IsValid() )
	{
		PreviousResponse = TargetBoundComponent->GetCollisionResponseToChannel( ECC_Pawn );
		TargetBoundComponent->SetCollisionResponseToChannel( ECC_Pawn, ECR_Ignore );
		Mass = TargetBoundComponent->GetMass();
		// 무거운 물체도 가볍게 움직일 수 있도록...
		TargetBoundComponent->SetMassOverrideInKg( NAME_None, Mass / 10.f, true );
		if ( OriginActor.IsValid() )
		{
			TargetBoundComponent->IgnoreActorWhenMoving( OriginActor.Get(), true );
		}

		TargetForwardVector = TargetBoundComponent->GetForwardVector();
		if ( const UNAKineticComponent* Component = OriginActor->GetComponentByClass<UNAKineticComponent>() )
		{
			OriginForwardVector = Component->GetActorForward();	
		}
	}
}

UNAAT_MoveActorTo::UNAAT_MoveActorTo(): Mass( 0.f ), PreviousResponse( ECR_MAX )
{
	bTickingTask = true;
}

void UNAAT_MoveActorTo::TickTask( float DeltaTime )
{
	Super::TickTask( DeltaTime );

	if ( OriginActor.IsValid() && TargetActor.IsValid() )
	{
		if ( UNAKineticComponent* KineticComponent = OriginActor->GetComponentByClass<UNAKineticComponent>() )
		{
			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor( OriginActor.Get() );
			Params.AddIgnoredActor( TargetActor.Get() );

			TArray<AActor*> OriginChildActors;
			TArray<AActor*> TargetChildActors;
			OriginActor->GetAllChildActors( OriginChildActors );
			TargetActor->GetAllChildActors( TargetChildActors );
			Params.AddIgnoredActors( OriginChildActors );
			Params.AddIgnoredActors( TargetChildActors );

			const FRotator RotationDelta = (KineticComponent->GetActorForward() - OriginForwardVector).Rotation();
			const FRotator TargetDelta = RotationDelta.RotateVector( FVector::ForwardVector ).Rotation();
			
			// Physics Handle Component를 사용하면 Lerp가 자동으로 적용됨
			//const FVector& Lerped = FMath::VInterpTo( TargetActor->GetActorLocation(), TargetPosition, DeltaTime, 10.f );

			FVector TargetLocation;
			FRotator TargetRotation;
			KineticComponent->GetTargetLocationAndRotation( TargetLocation, TargetRotation );

			const FRotator NewRotation = TargetRotation + TargetDelta * DeltaTime;

			FVector TargetPosition = UNAGA_KineticGrab::EvaluateActorPosition
			(
				OriginActor.Get(),
				TargetBoundComponent.Get(),
				KineticComponent->GetActorForward(),
				KineticComponent->GetGrabDistance()
			);

			if ( GetWorld()->LineTraceSingleByChannel( Hit, OriginActor->GetActorLocation(), TargetPosition, ECC_Visibility, Params ) )
			{
				Ability->CancelAbility( GetAbilitySpecHandle(), Ability->GetCurrentActorInfo(), Ability->GetCurrentActivationInfo(), false );
				return;
			}

			KineticComponent->SetTargetLocation( TargetPosition );
			KineticComponent->SetTargetRotation( NewRotation );
			//UE_LOG( LogTemp, Log, TEXT("%s"), *TargetPosition.ToString() );
			
			TargetForwardVector = TargetBoundComponent->GetForwardVector();
			OriginForwardVector = KineticComponent->GetActorForward();
		}
	}
}

void UNAAT_MoveActorTo::OnDestroy( bool bInOwnerFinished )
{
	Super::OnDestroy( bInOwnerFinished );

	if ( TargetBoundComponent.IsValid() )
	{
		TargetBoundComponent->SetCollisionResponseToChannel( ECC_Pawn, PreviousResponse );
		TargetBoundComponent->IgnoreActorWhenMoving( OriginActor.Get(), false );
		TargetBoundComponent->SetMassOverrideInKg( NAME_None, 0.f, false );
	}
}
