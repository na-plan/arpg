// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayAbility/NAGA_Suplex.h"
#include "NACharacter.h"
#include "AbilitySystemComponent.h"
#include "Combat/ActorComponent/NACombatComponent.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraActor.h"

UNAGA_Suplex::UNAGA_Suplex()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

}

void UNAGA_Suplex::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{

	if (HasAuthority(&ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}
	}

	if (UAbilitySystemComponent* ASC = ActorInfo->AvatarActor.Get()->FindComponentByClass<UAbilitySystemComponent>())
	{
		if (UNAMontageCombatComponent* CombatComponent = ActorInfo->AvatarActor->GetComponentByClass<UNAMontageCombatComponent>())
		{
			//ActorInfo->AbilitySystemComponent->PlayMontage
			//(
			//	this,
			//	ActivationInfo,
			//	CombatComponent->GetGrabMontage(),
			//	CombatComponent->GetMontagePlayRate()
			//);

			// Camera Action도 넣고 싶어지네 뭔가..
			ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor);
			Character->GetCharacterMovement()->DisableMovement();
			UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, CombatComponent->GetGrabMontage(), 1.0f);
			MontageTask->OnCompleted.AddDynamic(this, &UNAGA_Suplex::OnMontageFinished);
			MontageTask->ReadyForActivation();


			APlayerController* PlayerController = Cast<APlayerController>(ActorInfo->PlayerController);



			FVector StartLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
			FRotator StartRotation = PlayerController->PlayerCameraManager->GetCameraRotation();

			
			ACameraActor* ActionCamA = GetWorld()->SpawnActor<ACameraActor>(StartLocation, StartRotation);
			if (!ActionCamA) return;
			ACameraActor* ActionCamB = GetWorld()->SpawnActor<ACameraActor>(StartLocation, StartRotation);
			if (!ActionCamB) return;


			FVector LocationA = Character->GetActorLocation()+FVector(0,120,0);
			FRotator RotationA = FRotator(0, -90, 0);

			// Test용 바로 setting
			ActionCamA->SetActorLocationAndRotation(LocationA, RotationA);
			PlayerController->SetViewTargetWithBlend(ActionCamA, 1.f);


			FTimerHandle CameraBHandle;
			GetWorld()->GetTimerManager().SetTimer(CameraBHandle, FTimerDelegate::CreateLambda([=]()
				{
					FVector LocationB = LocationA + FVector(160, -120, -10);
					FRotator RotationB = FRotator(0, -180, 0);

					// B 카메라 전환
					ActionCamB->SetActorLocationAndRotation(LocationB, RotationB);
					PlayerController->SetViewTargetWithBlend(ActionCamB, 1.0f);				

				}), 1.0f, false); // 2초 뒤 실행

			FTimerHandle CameraBHandle2;
			GetWorld()->GetTimerManager().SetTimer(CameraBHandle2, FTimerDelegate::CreateLambda([=]()
				{
					ActionCamA->Destroy();
					ActionCamB->Destroy();
		

				}), 6.0f, false); // 2초 뒤 실행


			//UKismetSystemLibrary::MoveComponentTo(
			//	ActionCam,
			//	LocationA,
			//	RotationA,
			//	true, true,
			//	Overtime,
			//	true,
			//	EMoveComponentAction::Move,
			//	LatentInfo
			//);






			//PlayerController->SetViewTargetWithBlend(TempCam, 0.3f);


		}

	}
}

void UNAGA_Suplex::OnMontageFinished()
{
	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);


		APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());


		PlayerController->SetViewTargetWithBlend(GetAvatarActorFromActorInfo(), 0.3f);
		


	}
}
