// Fill out your copyright notice in the Description page of Project Settings.


#include "NASpectatorPawn.h"

#include "NAPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"


// Sets default values
ANASpectatorPawn::ANASpectatorPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = true;
	bUseControllerRotationRoll = true;
}

void ANASpectatorPawn::AttachToOtherPlayerImpl( const APawn* Other )
{
	if ( const ANACharacter* OtherCharacter = Cast<ANACharacter>( Other ) )
	{
		AttachToComponent
		(
			OtherCharacter->GetCameraBoom(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			USpringArmComponent::SocketName
		);

		TargetPawn = OtherCharacter;
		SetActorTickEnabled( true );
	}
}

// Called when the game starts or when spawned
void ANASpectatorPawn::BeginPlay()
{
	Super::BeginPlay();
	GetMovementComponent()->Deactivate();

	if ( HasAuthority() )
	{
		const AGameStateBase* GameState = GetWorld()->GetGameState();
		const ANAPlayerState* Other = nullptr;

		for ( const APlayerState* Element : GameState->PlayerArray )
		{
			if ( const ANAPlayerState* Casted = Cast<ANAPlayerState>( Element );
				 Casted && Casted->GetPlayerController() != GetController() && Casted->IsAlive() )
			{
				Other = Casted;
				break;
			}
		}

		if ( Other )
		{
			if ( HasAuthority() && IsLocallyControlled() )
			{
				AttachToOtherPlayerImpl( Other->GetPawn() );	
			}
			else
			{
				Client_AttachToOtherPlayer( Other->GetPawn() );	
			}
		}
	}
}

void ANASpectatorPawn::PossessedBy( AController* NewController )
{
	Super::PossessedBy( NewController );
	if ( IsLocallyControlled() )
	
		NewController->SetIgnoreLookInput( true );	
	}

void ANASpectatorPawn::UnPossessed()
{
	GetController()->SetIgnoreLookInput( false );
	Super::UnPossessed();
}

void ANASpectatorPawn::Client_AttachToOtherPlayer_Implementation( const APawn* OtherCharacter )
{
	AttachToOtherPlayerImpl( OtherCharacter );
}

// Called every frame
void ANASpectatorPawn::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if ( !TargetPawn.IsValid() )
	{
		SetActorTickEnabled( false );
	}

	if ( IsLocallyControlled() )
	{
		if ( const ANACharacter* Character = TargetPawn.Get() )
		{
			const FVector Location = Character->GetActorLocation();
			const FRotator Rotation = (Location - GetActorLocation()).GetSafeNormal().Rotation();
			const FRotator Lerp = FMath::RInterpConstantTo( GetActorRotation(), Rotation, DeltaTime, 50.f);
			if ( APlayerController* PC = Cast<APlayerController>( GetController() ) )
			{
				PC->SetControlRotation(Lerp);
			}
		}
	}
}

// Called to bind functionality to input
void ANASpectatorPawn::SetupPlayerInputComponent( UInputComponent* PlayerInputComponent )
{
	Super::SetupPlayerInputComponent( PlayerInputComponent );
}

