// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameAbilityTask/NAAT_WaitPlayerViewport.h"

#include "AbilitySystemComponent.h"

UNAAT_WaitPlayerViewport* UNAAT_WaitPlayerViewport::WaitPlayerViewport( UGameplayAbility* OwningAbility, FName TaskInstanceName,
                                                   UAbilitySystemComponent* CasterAbilitySystemComponent, AActor* InTargetShouldConcentrate )
{
	auto* ReturnValue = NewAbilityTask<UNAAT_WaitPlayerViewport>( OwningAbility, TaskInstanceName );
	ReturnValue->CasterAbilitySystemComponent = CasterAbilitySystemComponent;
	ReturnValue->TargetShouldConcentrate = InTargetShouldConcentrate;
	return ReturnValue;
}

void UNAAT_WaitPlayerViewport::TickTask( float DeltaTime )
{
	Super::TickTask( DeltaTime );

	if ( CasterAbilitySystemComponent && TargetShouldConcentrate )
	{
		if ( const APlayerController* PlayerController = CasterAbilitySystemComponent->AbilityActorInfo->PlayerController.Get() )
		{
			FVector2D ScreenPosition = FVector2D::ZeroVector;
			PlayerController->ProjectWorldLocationToScreen( TargetShouldConcentrate->GetActorLocation(), ScreenPosition, true );
		
			FVector CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
			FVector CameraForward = PlayerController->PlayerCameraManager->GetCameraRotation().Vector();
			FVector ToObject = ( TargetShouldConcentrate->GetActorLocation() - CameraLocation ).GetSafeNormal();

			const float Dot = FVector::DotProduct( CameraForward, ToObject );
			const bool bIsInFront = Dot > 0.f;
		
			if ( !bIsInFront )
			{
				CasterAbilitySystemComponent->CancelAbilityHandle( GetAbilitySpecHandle() );
				EndTask();
			}
		}
	}
	else
	{
		if ( CasterAbilitySystemComponent )
		{
			CasterAbilitySystemComponent->CancelAbilityHandle( GetAbilitySpecHandle() );
		}
		
		EndTask();
	}
}
