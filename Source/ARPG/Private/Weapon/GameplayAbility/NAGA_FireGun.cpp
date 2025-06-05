// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/GameplayAbility/NAGA_FireGun.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "Combat/Interface/NAHandActor.h"
#include "HP/GameplayEffect/NAGE_Damage.h"
#include "Weapon/GameplayEffect/NAGE_ConsumeAmmo.h"

UNAGA_FireGun::UNAGA_FireGun()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

bool UNAGA_FireGun::ConsumeAmmo(UAbilitySystemComponent* InAbilitySystemComponent)
{
	const FGameplayEffectContextHandle EffectContext = InAbilitySystemComponent->MakeEffectContext();
	const FGameplayEffectSpecHandle SpecHandle = InAbilitySystemComponent->MakeOutgoingSpec(UNAGE_ConsumeAmmo::StaticClass(), 1.f, EffectContext);
	const FActiveGameplayEffectHandle EffectHandle = InAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	return EffectHandle.WasSuccessfullyApplied();
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
	if ( HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo) )
	{
		if ( !CommitAbility(Handle, ActorInfo, ActivationInfo) )
		{
			EndAbility( Handle, ActorInfo, ActivationInfo, true, true );
		}
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
		}
		
		const FVector HeadLocation = ControlledPawn->GetComponentByClass<USkeletalMeshComponent>()->GetSocketLocation(INAHandActor::HeadSocketName);
		const FVector ForwardVector = ControlledPawn->GetActorForwardVector();
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
	}

	if ( UNAMontageCombatComponent* CombatComponent = ActorInfo->AvatarActor->GetComponentByClass<UNAMontageCombatComponent>() )
	{
		if ( const TScriptInterface<IAbilitySystemInterface> Interface = ActorInfo->OwnerActor.Get() )
		{
			Interface->GetAbilitySystemComponent()->PlayMontage( this, ActivationInfo, CombatComponent->GetMontage(), CombatComponent->GetMontagePlayRate() );
		}
	}
}

bool UNAGA_FireGun::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags)
{
	bool bResult = true;
	
	if (const TScriptInterface<INAHandActor> HandActor = ActorInfo->AvatarActor.Get())
	{
		bool HasAmmoConsumed = false;
		const TScriptInterface<IAbilitySystemInterface> Left = HandActor->GetLeftHandChildActorComponent()->GetChildActor();
		const TScriptInterface<IAbilitySystemInterface> Right = HandActor->GetRightHandChildActorComponent()->GetChildActor();
		
		if (Left)
		{
			HasAmmoConsumed = ConsumeAmmo(Left->GetAbilitySystemComponent());
		}
		if (Right)
		{
			if (!HasAmmoConsumed)
			{
				HasAmmoConsumed = ConsumeAmmo(Right->GetAbilitySystemComponent());
			}
		}

		bResult &= Left || Right;
		bResult &= HasAmmoConsumed;
	}

	return bResult;
}
