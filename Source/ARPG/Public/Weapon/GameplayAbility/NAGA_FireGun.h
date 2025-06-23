// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "NAGA_FireGun.generated.h"

class UNAMontageCombatComponent;
class INAHandActor;

UENUM( BlueprintType, meta=(Bitflags) )
enum class EHandActorSide : uint8
{
	None = 0,
	Left = 1 << 1,
	Right = 1 << 2,
};
ENUM_CLASS_FLAGS(EHandActorSide)

class UNAAT_WaitRotation;
/**
 * 
 */
UCLASS()
class ARPG_API UNAGA_FireGun : public UGameplayAbility
{
	GENERATED_BODY()

	UNAGA_FireGun();
	
	int32 GetRemainingAmmo( const UAbilitySystemComponent* InAbilitySystemComponent,
							const TSubclassOf<UGameplayEffect>& InAmmoType );

	static bool ConsumeAmmo(UAbilitySystemComponent* InAbilitySystemComponent, const TSubclassOf<UGameplayEffect>& InAmmoType);
	
	UFUNCTION()
	void OnRotationCompleted();

	void Fire();

	void FireOnce( UNAMontageCombatComponent* CombatComponent );

	static UNAMontageCombatComponent* GetCombatComponent( const UChildActorComponent* InChildActorComponent );

	UNAMontageCombatComponent* GetCombatComponent( EHandActorSide InHandActorSide ) const;
	
	UPROPERTY()
	UNAAT_WaitRotation* WaitRotationTask;

	EHandActorSide WhichHand;

	TScriptInterface<INAHandActor> CachedHandInterface;

protected:
	UFUNCTION()
	void OnMontageEnded( UAnimMontage* AnimMontage, bool bInterrupted );

	void CancelAbilityProxy( FGameplayTag GameplayTag, int Count );
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
};
