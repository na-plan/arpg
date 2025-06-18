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
		// 무거운 물체도 가볍게 움직일 수 있도록...
		TargetBoundComponent->SetMassOverrideInKg( NAME_None, TargetBoundComponent->GetMass() / 10.f, true );
		if ( OriginActor.IsValid() )
		{
			TargetBoundComponent->IgnoreActorWhenMoving( OriginActor.Get(), true );
		}

		AccumulatedQuat = FQuat::Identity;

		if ( const UNAKineticComponent* KineticComponent = OriginActor->GetComponentByClass<UNAKineticComponent>() )
		{
			PreviousOriginForwardQuat = KineticComponent->GetActorForward().ToOrientationQuat();
		}
	}
}

UNAAT_MoveActorTo::UNAAT_MoveActorTo(): PreviousResponse( ECR_MAX )
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

			// 오일러 방식으로 계산할 경우 짐벌락이 발생함!
			// 쿼터니언으로 두 각도의 차이를 계산함 (PQ_I * CQ)
			const FQuat RotationDelta = PreviousOriginForwardQuat.Inverse() * KineticComponent->GetActorForward().ToOrientationQuat();
			
			FVector ObjectLocation;
			FRotator ObjectRotation;
			KineticComponent->GetTargetLocationAndRotation( ObjectLocation, ObjectRotation );

			// PhysicsHandleComponent - 처음 그랩한 회전값을 기준으로 상대 회전 변환됨
			// 현재 회전각 값과 합성하여 새로운 다음 각도를 계산함
			AccumulatedQuat *= RotationDelta;
			const FQuat NewRotation = FMath::QInterpTo( ObjectRotation.Quaternion(), AccumulatedQuat, DeltaTime, 10.f );

			FVector TargetPosition = UNAGA_KineticGrab::EvaluateActorPosition
			(
				OriginActor.Get(),
				TargetBoundComponent.Get(),
				KineticComponent->GetActorForward(),
				KineticComponent->GetGrabDistance()
			);
			TargetPosition = FMath::VInterpTo( ObjectLocation, TargetPosition, DeltaTime, 10.f );

			if ( GetWorld()->LineTraceSingleByChannel( Hit, OriginActor->GetActorLocation(), TargetPosition, ECC_Visibility, Params ) )
			{
				Ability->CancelAbility( GetAbilitySpecHandle(), Ability->GetCurrentActorInfo(), Ability->GetCurrentActivationInfo(), false );
				return;
			}
			
			KineticComponent->SetTargetLocation( TargetPosition );
			KineticComponent->SetTargetRotation( NewRotation.Rotator() );

			const FRotator OriginForwardRotation = FMath::RInterpTo( OriginActor->GetActorRotation(), KineticComponent->GetActorForward().Rotation(), DeltaTime, 10.f );
			OriginActor->SetActorRotation( OriginForwardRotation );

			// 다음 연산에서 차이를 계산하기 위해 현재 회전값을 저장
			PreviousOriginForwardQuat = KineticComponent->GetActorForward().ToOrientationQuat();
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
