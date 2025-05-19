// Fill out your copyright notice in the Description page of Project Settings.

#include "DefaultAnimInstance.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UDefaultAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	APawn* Pawn = TryGetPawnOwner();
	if (GIsEditor && FApp::IsGame() && !Pawn)
	{
		checkf(false, TEXT("UDefaultAnimInstance를 사용하려면 소유권자가 Pawn이여야 합니다."));
		return;
	}
	else if (!Pawn) { return; }

	// TODO:: ADD Movement plz
	MovementComponent = Pawn->GetMovementComponent();
	check(MovementComponent);
}

void UDefaultAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!MovementComponent) { return; }

	Speed = UKismetMathLibrary::VSizeXY(MovementComponent->Velocity);

	bShoudMove = !FMath::IsNearlyZero(Speed);

	APawn* Pawn = TryGetPawnOwner();
	FRotator Rotation = Pawn->GetActorRotation();
	//
	//Direction = CalculateDirection(MovementComponent->Velocity, Rotation);



	//Player controller 부분입니다 필요시 해당 주석을 모두 풀고 사용하거나 필요 없을 경우 지워주세요
	//this is Player controller Parts 
	// if someone need this delete this Commets And Used it plz or someone do not need this delete all this codes
	/*
	//Pawn->GetController();
	//ABasePlayerController* OwnPlayerController = Cast<ABasePlayerController>(Pawn->GetController());
	//if (OwnPlayerController)
	//{
	//	ZoomAnim = OwnPlayerController->IsZoom;
	//}
	// 
	// 
	//Updating Rotation
	//Direction = AimRotation.Yaw;
	//if (Direction > 180.0) { Direction -= 360.0; }
	//else if (Direction < -180.0) { Direction += 360.0; }
	
	
	//쓸지 안쓸지 고민중 + fall anim not exist
	//bIsCrouch = MovementComponent->IsCrouching();	
	//bIsFalling = MovementComponent->IsFalling();
	*/
}
