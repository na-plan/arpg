// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/AbilityTask/NAAT_MoveActorTo.h"

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
		if ( OriginActor.IsValid() )
		{
			TargetBoundComponent->IgnoreActorWhenMoving( OriginActor.Get(), true );
		}
	}
}

UNAAT_MoveActorTo::UNAAT_MoveActorTo()
{
	bTickingTask = true;
}

void UNAAT_MoveActorTo::TickTask( float DeltaTime )
{
	Super::TickTask( DeltaTime );

	if ( OriginActor.IsValid() && TargetActor.IsValid() )
	{
		if ( const UNAKineticComponent* KineticComponent = OriginActor->GetComponentByClass<UNAKineticComponent>() )
		{
			const FVector TargetDimension = TargetBoundComponent->Bounds.BoxExtent * 2.f;
			FVector OriginOrigin, OriginExtents;
			OriginActor->GetActorBounds( true, OriginOrigin, OriginExtents );
			const FVector OriginDimension = OriginExtents * 2.f;

			const FVector ForwardVector = KineticComponent->GetActorForward();
			const FVector OriginDimensionToForward = OriginDimension * ForwardVector;
			const FVector TargetDimensionToForward = TargetDimension * ForwardVector;
			const FVector TotalDimensionToForward = OriginDimensionToForward + TargetDimensionToForward;
			const FVector ShouldDistanced = TotalDimensionToForward + ForwardVector * KineticComponent->GetGrabDistance();
			const FVector TargetPosition = OriginActor->GetActorLocation() + ShouldDistanced ;

			if ( !TargetActor->GetActorLocation().Equals( TargetPosition, 0.1f ) )
			{
				const FVector& Lerped = FMath::VInterpTo( OriginActor->GetActorLocation(), TargetPosition, DeltaTime, 10.f );
				
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
				
				if ( !GetWorld()->LineTraceSingleByChannel( Hit, OriginActor->GetActorLocation(), TargetPosition, ECC_Visibility, Params ) )
				{
					TargetActor->SetActorLocation( Lerped, false, nullptr, ETeleportType::TeleportPhysics );
				}
			}
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
	}
}
