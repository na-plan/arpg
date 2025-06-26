// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/GameplayAbility/NAGA_FireGun.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "Combat/Interface/NAHandActor.h"
#include "HP/GameplayEffect/NAGE_Damage.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/AbilityTask/NAAT_WaitRotation.h"

UNAGA_FireGun::UNAGA_FireGun(): WaitRotationTask( nullptr ), WhichHand()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

int32 UNAGA_FireGun::GetRemainingAmmo( const UAbilitySystemComponent* InAbilitySystemComponent, const TSubclassOf<UGameplayEffect>& InAmmoType )
{
	if ( InAbilitySystemComponent )
	{
		FGameplayEffectQuery Query;
		Query.EffectDefinition = InAmmoType;
		return InAbilitySystemComponent->GetActiveEffects( Query ).Num();
	}

	return 0;
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
	EndAbility( GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, bInterrupted );
}

void UNAGA_FireGun::CancelAbilityProxy( FGameplayTag GameplayTag, int Count )
{
	if ( Count >= 1 )
	{
		UE_LOG( LogTemp, Log, TEXT("[%hs]: Ability cancelled due to the failing tags"), __FUNCTION__ );
		CancelAbility( GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true );
	}
}

void UNAGA_FireGun::OnRotationCompleted()
{
	Fire();
}

void UNAGA_FireGun::Fire()
{
	const auto& Predicate = [ & ]( const EHandActorSide Check )
	{
		if ( EnumHasAnyFlags( WhichHand, Check ) )
		{
			AActor* TargetActor = nullptr;
			if ( EnumHasAnyFlags( WhichHand, EHandActorSide::Left ) )
			{
				TargetActor = CachedHandInterface->GetLeftHandChildActorComponent()->GetChildActor();
			}
			else if ( EnumHasAnyFlags( WhichHand, EHandActorSide::Right ) )
			{
				TargetActor = CachedHandInterface->GetRightHandChildActorComponent()->GetChildActor();
			}

			if ( TargetActor )
			{
				FireOnce( TargetActor->GetComponentByClass<UNAMontageCombatComponent>() );	
			}
		}
	};
	
	const TArray ToCheck { EHandActorSide::Left, EHandActorSide::Right };
	for ( const EHandActorSide Check : ToCheck )
	{
		Predicate( Check );
	}
}

void UNAGA_FireGun::FireOnce( UNAMontageCombatComponent* CombatComponent )
{
	if ( !CombatComponent )
	{
		return;
	}

	const FVector HeadLocation = GetActorInfo().AvatarActor->GetComponentByClass<USkeletalMeshComponent>()->GetSocketLocation(
		INAHandActor::HeadSocketName );
	const FVector ForwardVector = GetActorInfo().PlayerController->GetControlRotation().Vector().GetSafeNormal();
	const FVector EndLocation = HeadLocation + ForwardVector * 1000.f; // todo: Range 하드코딩

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor( GetActorInfo().AvatarActor.Get() );
	CollisionParams.AddIgnoredActor( CombatComponent->GetOwner() );
	
	FGameplayEffectContextHandle ContextHandle = GetCurrentActorInfo()->AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddInstigator(GetCurrentActorInfo()->OwnerActor.Get(), GetCurrentActorInfo()->AvatarActor.Get());
	ContextHandle.SetAbility(this);

	TArray<FHitResult> HitResults;
	float SphereSize = 30.f;
	FCollisionShape SweepShape = FCollisionShape::MakeSphere( SphereSize ); // 또는 MakeBox, MakeCapsule 등
	bool bMultiHit = GetWorld()->SweepMultiByChannel( HitResults, HeadLocation, EndLocation, FQuat::Identity, ECC_Pawn,
	                                                  SweepShape, CollisionParams );

	const FGameplayAbilityActivationInfo& Info = GetCurrentActivationInfo();
	if ( HasAuthority( &Info ) )
	{
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
	}

	FHitResult Result;
	bool bHit = GetWorld()->
		LineTraceSingleByProfile( Result, HeadLocation, EndLocation, TEXT( "FireGun" ), CollisionParams );
	bool FinalPrediction = bHit || Result.IsValidBlockingHit();

#if WITH_EDITOR || UE_BUILD_DEBUG
	DrawDebugLine( GetWorld(), HeadLocation, EndLocation, FinalPrediction ? FColor::Green : FColor::Red, false, 2.f );
#endif
	
	FGameplayCueParameters Parameters;
	Parameters.Instigator = GetCurrentActorInfo()->AvatarActor;
	Parameters.SourceObject = CombatComponent->GetOwner();
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

	if ( HasAuthority( &Info ))
	{
		if ( USkeletalMeshComponent* MeshComponent = GetCurrentActorInfo()->AvatarActor->GetComponentByClass<USkeletalMeshComponent>() )
		{
			if ( UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance() )
			{
				AnimInstance->Montage_GetEndedDelegate( CombatComponent->GetMontage() )->BindUObject( this, &UNAGA_FireGun::OnMontageEnded );
			}
		}
	}
}

UNAMontageCombatComponent* UNAGA_FireGun::GetCombatComponent( const UChildActorComponent* InChildActorComponent )
{
	if ( InChildActorComponent && InChildActorComponent->GetChildActor() )
	{
		return InChildActorComponent->GetChildActor()->GetComponentByClass<UNAMontageCombatComponent>();
	}

	return nullptr;
}

UNAMontageCombatComponent* UNAGA_FireGun::GetCombatComponent( EHandActorSide InHandActorSide ) const
{
	switch ( InHandActorSide )
	{
	case EHandActorSide::None:
		return nullptr;
	case EHandActorSide::Left:
		return GetCombatComponent( CachedHandInterface->GetLeftHandChildActorComponent() );
	case EHandActorSide::Right:
		return GetCombatComponent( CachedHandInterface->GetRightHandChildActorComponent() );
	}

	return nullptr;
}

void UNAGA_FireGun::ActivateAbility( const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo,
                                     const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData )
{
	if ( HasAuthorityOrPredictionKey( ActorInfo, &ActivationInfo ) )
	{
		if ( !CommitAbility( Handle, ActorInfo, ActivationInfo ) )
		{
			EndAbility( Handle, ActorInfo, ActivationInfo, true, true );
			return;
		}

		CachedHandInterface = ActorInfo->AvatarActor.Get();

		if ( HasAuthority( &ActivationInfo ) )
		{
			bool HasAmmoConsumed = false;
			const AActor* Left = CachedHandInterface->GetLeftHandChildActorComponent()->GetChildActor();
			const AActor* Right = CachedHandInterface->GetRightHandChildActorComponent()->GetChildActor();
				
			if ( Left != nullptr && EnumHasAnyFlags( WhichHand, EHandActorSide::Left ) )
			{
				const UNAMontageCombatComponent* LeftCombatComponent = Left->GetComponentByClass<UNAMontageCombatComponent>();
				HasAmmoConsumed = ConsumeAmmo(  ActorInfo->AbilitySystemComponent.Get(), LeftCombatComponent->GetAmmoType() );
			}
			if ( !HasAmmoConsumed && Right != nullptr && EnumHasAnyFlags( WhichHand, EHandActorSide::Right ) )
			{
				const UNAMontageCombatComponent* RightCombatComponent = Right->GetComponentByClass<UNAMontageCombatComponent>();
				ConsumeAmmo( ActorInfo->AbilitySystemComponent.Get(), RightCombatComponent->GetAmmoType() );
			}
		}

		const APawn* ControlledPawn = Cast<APawn>( ActorInfo->AvatarActor );
		UAbilitySystemComponent* AbilitySystemComponent = ControlledPawn->GetComponentByClass<UAbilitySystemComponent>();
		
		if ( !ControlledPawn || !AbilitySystemComponent )
		{
			EndAbility( Handle, ActorInfo, ActivationInfo, true, true );
			return;
		}
		
		FGameplayTagContainer FailingTags;
		FailingTags.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.KnockDown" ) );
		FailingTags.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.Dead" ) );
		FailingTags.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.Stun" ) );

		if ( AbilitySystemComponent->HasAnyMatchingGameplayTags( FailingTags ) )
		{
			EndAbility( Handle, ActorInfo, ActivationInfo, true, true );
			return;
		}

		const UNAMontageCombatComponent* CombatComponent = GetCombatComponent( WhichHand );
		
		const FVector PawnForwardVector = ControlledPawn->GetActorForwardVector();
		const FRotator ControlForwardVector = { 0, CombatComponent->GetAttackOrientation().Yaw, 0 };
		const float Delta = ControlForwardVector.Vector().Dot( PawnForwardVector );
		
		if ( !FMath::IsNearlyEqual( Delta, 1, 0.1f ) )
		{
			UE_LOG( LogTemp, Log, TEXT("[%hs]: Rotation required %d"), __FUNCTION__, GetWorld()->GetNetMode() );
			if ( WaitRotationTask )
			{
				WaitRotationTask->EndTask();
				WaitRotationTask = nullptr;
			}
			
			WaitRotationTask = UNAAT_WaitRotation::WaitRotation( this, TEXT( "WaitRotationForFireGun" ),
			                                                     ControlForwardVector.Quaternion() );
			WaitRotationTask->OnRotationCompleted.BindDynamic( this, &UNAGA_FireGun::OnRotationCompleted );
			WaitRotationTask->ReadyForActivation();
			
			if ( HasAuthority( &ActivationInfo ) )
			{
				AbilitySystemComponent->RegisterGameplayTagEvent(
					FGameplayTag::RequestGameplayTag( "Player.Status.KnockDown" ) ).AddUObject(
					this, &UNAGA_FireGun::CancelAbilityProxy );
				AbilitySystemComponent->RegisterGameplayTagEvent( FGameplayTag::RequestGameplayTag( "Player.Status.Dead" ) )
									  .AddUObject( this, &UNAGA_FireGun::CancelAbilityProxy );
				AbilitySystemComponent->RegisterGameplayTagEvent( FGameplayTag::RequestGameplayTag( "Player.Status.Stun" ) )
									  .AddUObject( this, &UNAGA_FireGun::CancelAbilityProxy );
			}
		}
		else
		{
			Fire();
		}	
	}
}

bool UNAGA_FireGun::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags)
{
	bool bResult = true;
	
	if ( const TScriptInterface<INAHandActor>& HandActor = ActorInfo->AvatarActor.Get() )
	{
		bool bCanConsume = false;
		const AActor* Left = HandActor->GetLeftHandChildActorComponent()->GetChildActor();
		const AActor* Right = HandActor->GetRightHandChildActorComponent()->GetChildActor();
		
		if ( Left )
		{
			const UNAMontageCombatComponent* LeftCombatComponent = Left->GetComponentByClass<UNAMontageCombatComponent>();
			bCanConsume = GetRemainingAmmo( ActorInfo->AbilitySystemComponent.Get(), LeftCombatComponent->GetAmmoType() ) > 0;
			if ( bCanConsume )
			{
				EnumAddFlags( WhichHand, EHandActorSide::Left );	
			}
		}
		if ( !bCanConsume && Right )
		{
			const UNAMontageCombatComponent* RightCombatComponent = Right->GetComponentByClass<UNAMontageCombatComponent>();
			bCanConsume = GetRemainingAmmo( ActorInfo->AbilitySystemComponent.Get(), RightCombatComponent->GetAmmoType() ) > 0;
			EnumAddFlags( WhichHand, EHandActorSide::Right );	
		}

		bResult &= Left || Right;
		bResult &= bCanConsume;
	}

	return bResult;
}

void UNAGA_FireGun::EndAbility( const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled )
{
	Super::EndAbility( Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled );

	WhichHand = EHandActorSide::None;
	
	if ( WaitRotationTask )
	{
		WaitRotationTask->EndTask();
		WaitRotationTask = nullptr;
	}
}
