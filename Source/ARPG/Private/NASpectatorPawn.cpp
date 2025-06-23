// Fill out your copyright notice in the Description page of Project Settings.


#include "NASpectatorPawn.h"

#include "NACharacter.h"
#include "NAPlayerState.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"


// Sets default values
ANASpectatorPawn::ANASpectatorPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCollisionComponent()->SetSimulatePhysics( false );

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 200.0f; // The camera follows at this distance behind the character
	CameraBoom->SocketOffset = FVector(0.f, 60.f, 80.f);
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bDoCollisionTest = false;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment( CameraBoom, USpringArmComponent::SocketName ); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->SetRelativeRotation(FRotator(-20.0f, 0.0f, 0.0f));
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

void ANASpectatorPawn::AttachToOtherPlayerImpl( APawn* Other )
{
	if ( ANACharacter* OtherCharacter = Cast<ANACharacter>( Other ) )
	{
		RootComponent->AttachToComponent
		(
			OtherCharacter->GetRootComponent(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale
		);
		RootComponent->SetRelativeLocation( { 0.f, 0.f, 50.f } );

		TargetPawn = OtherCharacter;
		SetActorTickEnabled( true );
	}
}

// Called when the game starts or when spawned
void ANASpectatorPawn::BeginPlay()
{
	Super::BeginPlay();
	GetMovementComponent()->Deactivate();

	if ( APlayerController* PC = Cast<APlayerController>( GetController() ) )
	{
		PC->SetViewTarget( this );
	}

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

void ANASpectatorPawn::Client_AttachToOtherPlayer_Implementation( APawn* OtherCharacter )
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
		return;
	}

	if ( IsLocallyControlled() )
	{
		FRotator ControllerDesiredRot = TargetPawn->GetReplicatedControlRotation();
		FRotator ControllerCurrentRot = Controller->GetControlRotation();
		FRotator ControllerSmoothedRot = FMath::RInterpTo(ControllerCurrentRot, ControllerDesiredRot, DeltaTime, 10.0f);

		if ( !ControllerSmoothedRot.Equals( ControllerCurrentRot, 0.1f ) )
		{
			Controller->SetControlRotation(ControllerSmoothedRot);
		}

		FVector Normal = (TargetPawn->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		FRotator DesiredRot = Normal.Rotation();
		FRotator SmoothRot = FMath::RInterpTo(ControllerCurrentRot, ControllerDesiredRot, DeltaTime, 10.0f);

		if ( !SmoothRot.Equals( GetActorRotation(), 0.1f ) )
		{
			SetActorRotation( DesiredRot );
		}
		
	}
}

// Called to bind functionality to input
void ANASpectatorPawn::SetupPlayerInputComponent( UInputComponent* PlayerInputComponent )
{
	Super::SetupPlayerInputComponent( PlayerInputComponent );
}

