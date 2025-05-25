// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/GameplayAbility/NAGA_FireGun.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Combat/Interface/NAHandActor.h"
#include "HP/GameplayEffect/NAGE_Damage.h"
#include "Weapon/GameplayEffect/NAGE_ConsumeAmmo.h"

bool UNAGA_FireGun::ConsumeAmmo(UAbilitySystemComponent* InAbilitySystemComponent)
{
	const FGameplayEffectContextHandle EffectContext = InAbilitySystemComponent->MakeEffectContext();
	const FGameplayEffectSpecHandle SpecHandle = InAbilitySystemComponent->MakeOutgoingSpec(UNAGE_ConsumeAmmo::StaticClass(), 1.f, EffectContext);
	const FActiveGameplayEffectHandle EffectHandle = InAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	return EffectHandle.WasSuccessfullyApplied();
}

void UNAGA_FireGun::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		}

		FGameplayEffectContextHandle ContextHandle = ActorInfo->AbilitySystemComponent->MakeEffectContext();
		ContextHandle.AddInstigator(ActorInfo->OwnerActor.Get(), ActorInfo->AvatarActor.Get());
		ContextHandle.SetAbility(this);

		const UWorld* World = ActorInfo->AvatarActor->GetWorld();
		const FVector HeadLocation = ActorInfo->AvatarActor->GetComponentByClass<USkeletalMeshComponent>()->GetSocketLocation(INAHandActor::HeadSocketName);
		const FVector ForwardVector = ActorInfo->AvatarActor->GetActorForwardVector();
		const FVector EndLocation = HeadLocation + ForwardVector * 1000.f; // todo: Range 하드코딩

		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(ActorInfo->AvatarActor.Get());

		if (FHitResult Result;
			World->LineTraceSingleByChannel(Result, HeadLocation, EndLocation, ECC_Pawn, CollisionParams))
		{
			ContextHandle.AddHitResult(Result, true);
			if (TScriptInterface<IAbilitySystemInterface> TargetInterface = Result.GetActor())
			{
				FGameplayEffectSpecHandle SpecHandle = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(UNAGE_Damage::StaticClass(), 1.f, ContextHandle);
				ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetInterface->GetAbilitySystemComponent());
			}
		}

		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
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
