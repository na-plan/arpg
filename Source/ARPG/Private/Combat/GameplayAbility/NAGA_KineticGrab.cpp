// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/GameplayAbility/NAGA_KineticGrab.h"

#include "AbilitySystemComponent.h"
#include "NACharacter.h"
#include "Combat/AbilityTask/NAAT_ConsumeKineticGrabAP.h"
#include "Combat/AbilityTask/NAAT_MoveActorTo.h"
#include "Combat/AttributeSet/NAKineticAttributeSet.h"
#include "Combat/PhysicsConstraintComponent/NAKineticComponent.h"

FVector UNAGA_KineticGrab::EvaluateActorPosition( const AActor* OriginActor, const UPrimitiveComponent* TargetBoundComponent, const FVector& ForwardVector, float Distance )
{
	const FVector ShouldDistanced = GetMinimumDistance( OriginActor, TargetBoundComponent, ForwardVector ) + ( ForwardVector * Distance );
	const FVector OriginPosition = OriginActor->GetActorLocation();
	const FVector TargetPosition = OriginPosition + ShouldDistanced;

	return TargetPosition;
}

FVector UNAGA_KineticGrab::EvaluateActorPosition( const AActor* OriginActor, const FVector& ForwardVector, const float MinimumDistance )
{
	const FVector ShouldDistanced = ForwardVector * MinimumDistance;
	const FVector OriginPosition = OriginActor->GetActorLocation();
	const FVector TargetPosition = OriginPosition + ShouldDistanced;

	return TargetPosition;
}

FVector UNAGA_KineticGrab::GetMinimumDistance( const AActor* OriginActor,
                                               const UPrimitiveComponent* TargetBoundComponent, const FVector& ForwardVector )
{
	const FVector TargetDimension = TargetBoundComponent->Bounds.BoxExtent * 2.f;
	FVector OriginOrigin, OriginExtents;
	OriginActor->GetActorBounds( true, OriginOrigin, OriginExtents );
	const FVector OriginDimension = OriginExtents * 2.f;

	const FVector OriginDimensionToForward = OriginDimension * ForwardVector;
	const FVector TargetDimensionToForward = TargetDimension * ForwardVector;

	return TargetDimensionToForward + OriginDimensionToForward;
}

bool UNAGA_KineticGrab::CommitAbility( const FGameplayAbilitySpecHandle Handle,
                                       const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                       FGameplayTagContainer* OptionalRelevantTags )
{
	bool bResult = Super::CommitAbility( Handle, ActorInfo, ActivationInfo, OptionalRelevantTags );

	if ( bResult )
	{
		const UNAKineticAttributeSet* AttributeSet = Cast<UNAKineticAttributeSet>( ActorInfo->AbilitySystemComponent->GetAttributeSet( UNAKineticAttributeSet::StaticClass() ) );
		bResult &= AttributeSet != nullptr;
		
		if ( AttributeSet )
		{
			bResult &= AttributeSet->GetAP() > 0.f;
		}
	}

	return bResult;
}

void UNAGA_KineticGrab::EndAbility( const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled )
{
	Super::EndAbility( Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled );

	if ( HasAuthority( &ActivationInfo ) )
	{
		UNAKineticComponent* Component = ActorInfo->AvatarActor->GetComponentByClass<UNAKineticComponent>();
		check( Component );

		if ( Component && Component->bIsGrab )
		{
			Component->ReleaseComponent();
			Component->bIsGrab = false;
		}

		if ( APConsumeTask )
		{
			APConsumeTask->OnAPDepleted.RemoveAll( this );
			APConsumeTask->EndTask();
			APConsumeTask = nullptr;
		}

		if ( MoveActorToTask )
		{
			MoveActorToTask->EndTask();
			MoveActorToTask = nullptr;
		}
	}
}

void UNAGA_KineticGrab::CancelAbility( const FGameplayAbilitySpecHandle Handle,
                                       const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                       bool bReplicateCancelAbility )
{
	Super::CancelAbility( Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility );
}

void UNAGA_KineticGrab::OnAPDepleted()
{
	EndAbility( GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false );
}

void UNAGA_KineticGrab::ActivateAbility( const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                         const FGameplayEventData* TriggerEventData )
{
	Super::ActivateAbility( Handle, ActorInfo, ActivationInfo, TriggerEventData );

	if ( HasAuthority( &ActivationInfo ) )
	{
		if ( !CommitAbility( Handle, ActorInfo, ActivationInfo ) )
		{
			EndAbility( Handle, ActorInfo, ActivationInfo, false, true );
			return;
		}

		bool bSuccess = false;

		FHitResult Hit;
		UNAKineticComponent* Component = ActorInfo->AvatarActor->GetComponentByClass<UNAKineticComponent>();
		
		if ( Component )
		{
			if ( const APawn* Pawn = Cast<APawn>( ActorInfo->AvatarActor ) )
			{
				const FVector StartLocation = ActorInfo->AvatarActor->GetActorLocation();
				const FVector ForwardVector = Pawn->GetControlRotation().Vector();
				const FVector EndLocation = StartLocation + ForwardVector * Component->GetRange();

				TArray<AActor*> ChildActors;
				ActorInfo->AvatarActor->GetAllChildActors( ChildActors );
		
				FCollisionQueryParams Params;
				Params.AddIgnoredActor( ActorInfo->AvatarActor.Get() );
				Params.AddIgnoredActors( ChildActors );

				const bool bHit = GetWorld()->LineTraceSingleByChannel
				(
					Hit,
					StartLocation,
					EndLocation,
					ECC_WorldDynamic,
					Params
				);

#if WITH_EDITOR || UE_BUILD_DEBUG
				DrawDebugLine
				(
					GetWorld(),
					StartLocation,
					EndLocation,
					bHit ? FColor::Green : FColor::Red,
					false,
					2.f
				);
#endif	
			}
		}

		if ( Hit.IsValidBlockingHit() )
		{
			if ( Hit.GetComponent()->IsSimulatingPhysics() )
			{
				Component->GrabComponentAtLocationWithRotation
				(
					Hit.GetComponent(),
					NAME_None,
					EvaluateActorPosition
					(
						ActorInfo->AvatarActor.Get(),
						Hit.GetComponent(),
						Component->GetActorForward(),
						Component->GetGrabDistance()
					),
					Hit.GetComponent()->GetComponentRotation()
				);
				Component->bIsGrab = true;
				bSuccess = true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Object %s physics is not simulated, will not grab."), *GetNameSafe( Hit.GetActor() ) )
			}
		}

		if ( !bSuccess )
		{
			EndAbility( Handle, ActorInfo, ActivationInfo, false, true );
		}
		else
		{
			MoveActorToTask = UNAAT_MoveActorTo::MoveActorTo( this, TEXT( "MoveActorTo" ), ActorInfo->AvatarActor.Get(), Hit.GetActor(), Hit.GetComponent() );
			MoveActorToTask->ReadyForActivation();
			
			APConsumeTask = UNAAT_ConsumeKineticGrabAP::WaitAPDepleted( this, TEXT( "CheckDepleted" ) );
			APConsumeTask->OnAPDepleted.AddUObject( this, &UNAGA_KineticGrab::OnAPDepleted );
			APConsumeTask->ReadyForActivation();
		}
	}
}

void UNAGA_KineticGrab::InputReleased( const FGameplayAbilitySpecHandle Handle,
                                       const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo )
{
	EndAbility( Handle, ActorInfo, ActivationInfo, false, false );
}
