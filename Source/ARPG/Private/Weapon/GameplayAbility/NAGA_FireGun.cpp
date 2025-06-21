// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/GameplayAbility/NAGA_FireGun.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "Combat/Interface/NAHandActor.h"
#include "HP/GameplayEffect/NAGE_Damage.h"
#include "Weapon/AbilityTask/NAGA_WaitRotation.h"

UNAGA_FireGun::UNAGA_FireGun(): WaitRotationTask( nullptr )
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	AbilityTags.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.Firing" ) );
}

bool UNAGA_FireGun::ConsumeAmmo( UAbilitySystemComponent* InAbilitySystemComponent,
                                 const TSubclassOf<UGameplayEffect>& InAmmoType )
{
	if ( InAbilitySystemComponent )
	{
		FGameplayEffectQuery Query;
		Query.EffectDefinition = InAmmoType;
		const int32 Consumed = InAbilitySystemComponent->RemoveActiveEffects( Query, 1 );
		return Consumed != 0;
	}

	return false;
}

void UNAGA_FireGun::OnMontageEnded( UAnimMontage* AnimMontage, bool bInterrupted )
{
	if ( const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo() )
	{
		bool bSuccessfullyEnded = false;
		
		if ( UNAMontageCombatComponent* CombatComponent = ActorInfo->AvatarActor->GetComponentByClass<UNAMontageCombatComponent>() )
		{
			if ( CombatComponent->GetMontage() == AnimMontage && !bInterrupted )
			{
				bSuccessfullyEnded = true;
				EndAbility( GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, false );
			}
		}

		if ( USkeletalMeshComponent* MeshComponent = ActorInfo->OwnerActor->GetComponentByClass<USkeletalMeshComponent>() )
		{
			if ( UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance() )
			{
				AnimInstance->OnMontageEnded.RemoveAll( this );
			}
		}
		
		if ( !bSuccessfullyEnded )
		{
			EndAbility( GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true );
		}
	}
}

void UNAGA_FireGun::CancelAbilityProxy( FGameplayTag GameplayTag, int Count )
{
	if ( Count >= 1 )
	{
		CancelAbility( GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false );
	}
}

void UNAGA_FireGun::OnRotationCompleted()
{
	APawn* ControlledPawn = Cast<APawn>( GetCurrentActorInfo()->OwnerActor );
	
	if ( !ControlledPawn )
	{
		EndAbility( GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true );
		return;
	}
	
	Fire( ControlledPawn );
}

void UNAGA_FireGun::Fire( APawn* ControlledPawn )
{
	const UWorld* World = GetCurrentActorInfo()->OwnerActor->GetWorld();

	UNAMontageCombatComponent* CombatComponent = GetCurrentActorInfo()->AvatarActor->GetComponentByClass<
		UNAMontageCombatComponent>();
	if ( !CombatComponent )
	{
		EndAbility( GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true );
		return;
	}

	const FVector HeadLocation = ControlledPawn->GetComponentByClass<USkeletalMeshComponent>()->GetSocketLocation(
		INAHandActor::HeadSocketName );
	const FVector ForwardVector = ControlledPawn->Controller->GetControlRotation().Vector().GetSafeNormal();
	const FVector EndLocation = HeadLocation + ForwardVector * 1000.f; // todo: Range 하드코딩

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor( ControlledPawn );

	FHitResult Result;
	//bool bHit = World->LineTraceSingleByChannel( Result, HeadLocation, EndLocation, ECC_Pawn, CollisionParams );

	FGameplayEffectContextHandle ContextHandle = GetCurrentActorInfo()->AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddInstigator(GetCurrentActorInfo()->OwnerActor.Get(), GetCurrentActorInfo()->AvatarActor.Get());
	ContextHandle.SetAbility(this);

	TArray<FHitResult> HitResults;
	float SphereSize = 30.f;
	FCollisionShape SweepShape = FCollisionShape::MakeSphere( SphereSize ); // 또는 MakeBox, MakeCapsule 등
	bool bMultiHit = GetWorld()->SweepMultiByChannel( HitResults, HeadLocation, EndLocation, FQuat::Identity, ECC_Pawn,
	                                                  SweepShape, CollisionParams );
	if ( !HitResults.IsEmpty() )
	{
		for ( const FHitResult HitResult : HitResults )
		{
			if ( TScriptInterface<IAbilitySystemInterface> TargetInterface = HitResult.GetActor() )
			{
				FGameplayEffectSpecHandle SpecHandle = GetCurrentActorInfo()->AbilitySystemComponent->MakeOutgoingSpec(
					UNAGE_Damage::StaticClass(), 1.f, ContextHandle );
				SpecHandle.Data->SetSetByCallerMagnitude( FGameplayTag::RequestGameplayTag( TEXT( "Data.Damage" ) ),
				                                          -CombatComponent->GetBaseDamage() );
				GetCurrentActorInfo()->AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
					*SpecHandle.Data.Get(), TargetInterface->GetAbilitySystemComponent() );
#if WITH_EDITOR || UE_BUILD_DEBUG
				DrawDebugSphere(
					GetWorld(),
					HitResult.ImpactPoint, // 또는 Hit.Location
					SphereSize,
					12,
					FColor::Green,
					false,
					2.0f,
					0,
					1.5f
				);
#endif
			}
		}
	}
	bool bHit = World->
		LineTraceSingleByProfile( Result, HeadLocation, EndLocation, TEXT( "FireGun" ), CollisionParams );
	bool FinalPrediction = bHit || Result.IsValidBlockingHit();

#if WITH_EDITOR || UE_BUILD_DEBUG
	DrawDebugLine( GetWorld(), HeadLocation, EndLocation, FinalPrediction ? FColor::Green : FColor::Red, false, 2.f );
#endif

	if ( USkeletalMeshComponent* MeshComponent = ControlledPawn->GetComponentByClass<USkeletalMeshComponent>() )
	{
		if ( UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance() )
		{
			AnimInstance->OnMontageEnded.AddUniqueDynamic( this, &UNAGA_FireGun::OnMontageEnded );
		}
	}

	FGameplayCueParameters Parameters;
	Parameters.Instigator = GetAvatarActorFromActorInfo();
	if ( FinalPrediction )
	{
		Parameters.Normal = Result.Normal;
		Parameters.Location = Result.Location;
	}
	else
	{
		Parameters.Normal = ForwardVector;
		Parameters.Location = EndLocation;
	}
	
	GetCurrentActorInfo()->AbilitySystemComponent->ExecuteGameplayCue(
		FGameplayTag::RequestGameplayTag( TEXT( "GameplayCue.Gun.Fire" ) ), Parameters );
}

void UNAGA_FireGun::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if ( !CommitAbility(Handle, ActorInfo, ActivationInfo) )
	{
		EndAbility( Handle, ActorInfo, ActivationInfo, true, true );
		return;
	}

	if ( HasAuthority( &ActivationInfo ) )
	{
		APawn* ControlledPawn = Cast<APawn>( ActorInfo->OwnerActor );
		UAbilitySystemComponent* AbilitySystemComponent = ControlledPawn->GetComponentByClass<UAbilitySystemComponent>();

		if ( !ControlledPawn || !AbilitySystemComponent )
		{
			EndAbility( Handle, ActorInfo, ActivationInfo, false, true );
			return;
		}

		FGameplayTagContainer FailingTags;
		FailingTags.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.KnockDown" ) );
		FailingTags.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.Dead" ) );
		FailingTags.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.Stun" ) );
		
		if ( AbilitySystemComponent->HasAnyMatchingGameplayTags( FailingTags ) )
		{
			EndAbility( Handle, ActorInfo, ActivationInfo, false, true );
			return;
		}

		const FRotator PawnForward = ControlledPawn->GetActorForwardVector().Rotation();
		const FRotator ControlForward = ControlledPawn->GetController()->GetControlRotation();
		
		if ( !FMath::IsNearlyEqual( PawnForward.Yaw, ControlForward.Yaw, 0.01 ) )
		{
			WaitRotationTask = UNAAT_WaitRotation::WaitRotation( this, TEXT("WaitRotationForFireGun"), ControlForward.Quaternion() );
			WaitRotationTask->ReadyForActivation();
			WaitRotationTask->OnRotationCompleted.BindDynamic( this, &UNAGA_FireGun::OnRotationCompleted );

			AbilitySystemComponent->RegisterGameplayTagEvent( FGameplayTag::RequestGameplayTag( "Player.Status.KnockDown" ) ).AddUObject( this, &UNAGA_FireGun::CancelAbilityProxy );
			AbilitySystemComponent->RegisterGameplayTagEvent( FGameplayTag::RequestGameplayTag( "Player.Status.Dead" ) ).AddUObject( this, &UNAGA_FireGun::CancelAbilityProxy );
			AbilitySystemComponent->RegisterGameplayTagEvent( FGameplayTag::RequestGameplayTag( "Player.Status.Stun" ) ).AddUObject( this, &UNAGA_FireGun::CancelAbilityProxy );
		}
		else
		{
			Fire( ControlledPawn );
		}
	}
}

bool UNAGA_FireGun::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags)
{
	bool bResult = true;
	
	if ( const TScriptInterface<INAHandActor> HandActor = ActorInfo->OwnerActor.Get() )
	{
		bool HasAmmoConsumed = false;
		const AActor* Left = HandActor->GetLeftHandChildActorComponent()->GetChildActor();
		const AActor* Right = HandActor->GetRightHandChildActorComponent()->GetChildActor();

		const TScriptInterface<IAbilitySystemInterface> AbilityInterface = ActorInfo->OwnerActor.Get();
		
		if ( Left )
		{
			const UNAMontageCombatComponent* LeftCombatComponent = Left->GetComponentByClass<UNAMontageCombatComponent>();
			HasAmmoConsumed = ConsumeAmmo( AbilityInterface->GetAbilitySystemComponent(), LeftCombatComponent->GetAmmoType() );
		}
		if ( !HasAmmoConsumed && Right )
		{
			const UNAMontageCombatComponent* RightCombatComponent = Right->GetComponentByClass<UNAMontageCombatComponent>();
			HasAmmoConsumed = ConsumeAmmo( AbilityInterface->GetAbilitySystemComponent(), RightCombatComponent->GetAmmoType() );
		}

		bResult &= Left || Right;
		bResult &= HasAmmoConsumed;
	}

	return bResult;
}

void UNAGA_FireGun::EndAbility( const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled )
{
	Super::EndAbility( Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled );

	if ( WaitRotationTask )
	{
		WaitRotationTask->EndTask();
		WaitRotationTask = nullptr;
	}
}
