// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/AbilityTask/NAAT_MoveActorTo.h"

#include "Combat/GameplayAbility/NAGA_KineticGrab.h"
#include "Combat/PhysicsConstraintComponent/NAKineticComponent.h"

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
		TargetBoundComponent->SetMassOverrideInKg( NAME_None, FMath::LogX( 10, Mass ), true );
		if ( OriginActor.IsValid() )
		{
			TargetBoundComponent->IgnoreActorWhenMoving( OriginActor.Get(), true );
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
			FVector TargetPosition = UNAGA_KineticGrab::EvaluateActorPosition
			(
				OriginActor.Get(),
				TargetBoundComponent.Get(),
				KineticComponent->GetActorForward(),
				KineticComponent->GetGrabDistance()
			);

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

			if ( GetWorld()->LineTraceSingleByChannel( Hit, OriginActor->GetActorLocation(), TargetPosition, ECC_Visibility, Params ) )
			{
				Ability->CancelAbility( GetAbilitySpecHandle(), Ability->GetCurrentActorInfo(), Ability->GetCurrentActivationInfo(), false );
				return;
			}
			
			//const FVector& Lerped = FMath::VInterpTo( TargetActor->GetActorLocation(), TargetPosition, DeltaTime, 10.f );
			KineticComponent->SetTargetLocationAndRotation( TargetPosition, KineticComponent->GetActorForward().Rotation() );
			UE_LOG( LogTemp, Log, TEXT("%s"), *TargetPosition.ToString() );
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
