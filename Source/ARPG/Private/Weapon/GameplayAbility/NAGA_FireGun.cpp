// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/GameplayAbility/NAGA_FireGun.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "Combat/Interface/NAHandActor.h"
#include "HP/GameplayEffect/NAGE_Damage.h"

UNAGA_FireGun::UNAGA_FireGun()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
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

void UNAGA_FireGun::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if ( !CommitAbility(Handle, ActorInfo, ActivationInfo) )
	{
		EndAbility( Handle, ActorInfo, ActivationInfo, true, true );
		return;
	}

	UNAMontageCombatComponent* CombatComponent = ActorInfo->AvatarActor->GetComponentByClass<UNAMontageCombatComponent>();
	if ( !CombatComponent )
	{
		EndAbility( Handle, ActorInfo, ActivationInfo, true, true );
		return;
	}

	if ( HasAuthority( &ActivationInfo ) )
	{
		FGameplayEffectContextHandle ContextHandle = ActorInfo->AbilitySystemComponent->MakeEffectContext();
		ContextHandle.AddInstigator(ActorInfo->OwnerActor.Get(), ActorInfo->AvatarActor.Get());
		ContextHandle.SetAbility(this);

		const UWorld* World = ActorInfo->OwnerActor->GetWorld();
		const APawn* ControlledPawn = Cast<APawn>( ActorInfo->OwnerActor );

		if ( !ControlledPawn )
		{
			EndAbility( Handle, ActorInfo, ActivationInfo, true, false );
			return;
		}
		
		const FVector HeadLocation = ControlledPawn->GetComponentByClass<USkeletalMeshComponent>()->GetSocketLocation(INAHandActor::HeadSocketName);
		const FVector ForwardVector = ControlledPawn->Controller->GetControlRotation().Vector().GetSafeNormal();
		const FVector EndLocation = HeadLocation + ForwardVector * 1000.f; // todo: Range 하드코딩

		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor( ControlledPawn );
		CollisionParams.AddIgnoredActor( ActorInfo->AvatarActor.Get() );

		FHitResult Result;
		bool bHit = World->LineTraceSingleByChannel( Result, HeadLocation, EndLocation, ECC_Pawn, CollisionParams );
		bool FinalPrediction = bHit || Result.IsValidBlockingHit();
		if ( FinalPrediction )
		{
			ContextHandle.AddHitResult(Result, true);
			if ( TScriptInterface<IAbilitySystemInterface> TargetInterface = Result.GetActor() )
			{
				FGameplayEffectSpecHandle SpecHandle = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(UNAGE_Damage::StaticClass(), 1.f, ContextHandle);
				SpecHandle.Data->SetSetByCallerMagnitude( FGameplayTag::RequestGameplayTag( TEXT( "Data.Damage" ) ), -CombatComponent->GetBaseDamage() );
				ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetInterface->GetAbilitySystemComponent());
			}
		}

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
		ActorInfo->AbilitySystemComponent->ExecuteGameplayCue( FGameplayTag::RequestGameplayTag( TEXT( "GameplayCue.Gun.Fire" ) ), Parameters );
	}

	if ( const TScriptInterface<IAbilitySystemInterface> Interface = ActorInfo->OwnerActor.Get() )
	{
		Interface->GetAbilitySystemComponent()->PlayMontage( this, ActivationInfo, CombatComponent->GetMontage(), CombatComponent->GetMontagePlayRate() );
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
