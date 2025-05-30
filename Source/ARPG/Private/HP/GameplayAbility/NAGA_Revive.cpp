// Fill out your copyright notice in the Description page of Project Settings.


#include "HP/GameplayAbility/NAGA_Revive.h"

#include "AbilitySystemComponent.h"
#include "NACharacter.h"
#include "Ability/AttributeSet/NAAttributeSet.h"
#include "Ability/GameAbilityTask/NAAT_WaitPlayerViewport.h"
#include "GameFramework/GameStateBase.h"
#include "HP/ActorComponent/NAVitalCheckComponent.h"
#include "HP/GameplayEffect/NAGE_Damage.h"
#include "HP/GameplayEffect/NAGE_Dead.h"
#include "HP/GameplayEffect/NAGE_Helping.h"
#include "HP/GameplayEffect/NAGE_Revive.h"
#include "Net/UnrealNetwork.h"

void UNAGA_Revive::OnReviveSucceeded()
{
	if ( const ANACharacter* Character = RevivingTarget.Get() )
	{
		FGameplayEffectQuery Query;
		Query.EffectDefinition = UNAGE_Damage::StaticClass();
		const TArray<FActiveGameplayEffectHandle> OutResults = Character->GetAbilitySystemComponent()->GetActiveEffects( Query );

		// 지금까지 입었던 모든 데미지 효과를 제거함
		for ( const FActiveGameplayEffectHandle& Handle : OutResults )
		{
			Character->GetAbilitySystemComponent()->RemoveActiveGameplayEffect( Handle );
		}

		EndAbility( GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false );
	}
}

void UNAGA_Revive::OnHitWhileRevive( const FOnAttributeChangeData& OnAttributeChangeData )
{
	// 데미지를 입거나 쓰러지면 부활을 취소
	if ( OnAttributeChangeData.NewValue < OnAttributeChangeData.OldValue ||
		 OnAttributeChangeData.NewValue <= 0.f )
	{
		CancelAbility( GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false );
	}
}

void UNAGA_Revive::CheckKnockDownDead( UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle )
{
	if ( GameplayEffectSpec.Def->IsA<UNAGE_Dead>() )
	{
		CancelAbility( GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false );
	}
}

void UNAGA_Revive::ActivateAbility( const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData )
{
	Super::ActivateAbility( Handle, ActorInfo, ActivationInfo, TriggerEventData );

	if ( HasAuthority( &ActivationInfo ) )
	{
		if ( !CommitAbility( Handle, ActorInfo, ActivationInfo ) )
		{
			UE_LOG( LogAbilitySystemComponent, Log, TEXT("%hs: Cannot revive anything"), __FUNCTION__ );
			EndAbility( Handle, ActorInfo, ActivationInfo, false, true );
			return;
		}

		if ( const USkeletalMeshComponent* SkeletalMeshComponent = ActorInfo->AvatarActor->GetComponentByClass<USkeletalMeshComponent>() )
		{
			const FVector StartLocation = SkeletalMeshComponent->GetSocketLocation( "HeadSocket" );
			const FVector ForwardVector = ActorInfo->PlayerController->GetControlRotation().Vector();
			constexpr float Length = 100.f;
			const FVector EndLocation = StartLocation + (ForwardVector * Length);

			TArray<FHitResult> OutResult;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor( ActorInfo->AvatarActor.Get() );
			const bool bHit = ActorInfo->AvatarActor->GetWorld()->LineTraceMultiByChannel( OutResult, StartLocation, EndLocation, ECC_Pawn, Params );

			if ( bHit || OutResult.Num() > 0 )
			{
				for ( const FHitResult& Result : OutResult )
				{
					ANACharacter* Character = Cast<ANACharacter>( Result.GetActor() );
					if ( !Character )
					{
						continue;
					}

					UAbilitySystemComponent* RevivingASC = Character->GetAbilitySystemComponent();
					UAbilitySystemComponent* HelperASC = ActorInfo->AbilitySystemComponent.Get();

					// 누운 상태가 아님
					if ( !RevivingASC->HasMatchingGameplayTag( FGameplayTag::RequestGameplayTag( "Player.Status.KnockDown" ) ) )
					{
						continue;
					}
					
					// 누군가 이미 살려주고 있음
					if ( RevivingASC->HasMatchingGameplayTag( FGameplayTag::RequestGameplayTag( "Player.Status.Reviving" ) ) )
					{
						continue;
					}

					RevivingTarget = Character;

					// 살려주는 중
					FGameplayEffectContextHandle HelperContextHandle = HelperASC->MakeEffectContext();
					HelperContextHandle.AddInstigator( ActorInfo->OwnerActor.Get(), ActorInfo->AvatarActor.Get() );
					HelperContextHandle.SetAbility( this );
					HelperContextHandle.AddSourceObject( this );
					FGameplayEffectSpecHandle HelperEffectHandle = HelperASC->MakeOutgoingSpec( UNAGE_Helping::StaticClass(), 1.f, HelperContextHandle );
					HelperASC->ApplyGameplayEffectSpecToSelf( *HelperEffectHandle.Data.Get() );
					
					if ( RevivingTarget.IsValid() )
					{
						// 부활 중
						FGameplayEffectContextHandle RevivingContextHandle = RevivingASC->MakeEffectContext();
						RevivingContextHandle.AddInstigator( ActorInfo->OwnerActor.Get(), ActorInfo->AvatarActor.Get() );
						RevivingContextHandle.SetAbility( this );
						RevivingContextHandle.AddSourceObject( this );
						FGameplayEffectSpecHandle RevivingEffectHandle = RevivingASC->MakeOutgoingSpec( UNAGE_Revive::StaticClass(), 1.f, HelperContextHandle );
						RevivingASC->ApplyGameplayEffectSpecToSelf( *RevivingEffectHandle.Data.Get() );
						
						// 부활받던 사람이 중간에 죽을 경우
						RevivingTargetHandle = RevivingASC->OnGameplayEffectAppliedDelegateToSelf.AddUObject( this, &UNAGA_Revive::CheckKnockDownDead );
					}

					// 부활시키는 캐릭터가 화면 밖으로 나가면 취소
					ViewportCheckTask = UNAAT_WaitPlayerViewport::WaitPlayerViewport( this, "WaitPlayerViewport", ActorInfo->AbilitySystemComponent.Get(), Character );
					ViewportCheckTask->ReadyForActivation();

					ActorInfo->AvatarActor->GetWorld()->GetTimerManager().SetTimer( ReviveTimerHandle, this, &UNAGA_Revive::OnReviveSucceeded, 5.f, false );
					
					// 부활해주는 사람이 중간에 맞거나 눕는 경우
					ReviveCancelHandle = ActorInfo->AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate( UNAAttributeSet::GetHealthAttribute() ).AddUObject( this, &UNAGA_Revive::OnHitWhileRevive );
					break;
				}
			}
			else
			{
				UE_LOG( LogAbilitySystemComponent, Log, TEXT("%hs: Unable to reviving target"), __FUNCTION__ );
			}
		}
	}
}

void UNAGA_Revive::CancelAbility( const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility )
{
	Super::CancelAbility( Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility );
	UE_LOG( LogAbilitySystemComponent, Log, TEXT("%hs: Ability cancelled"), __FUNCTION__ );
}

void UNAGA_Revive::EndAbility( const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled )
{
	Super::EndAbility( Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled );
	UE_LOG( LogAbilitySystemComponent, Log, TEXT("%hs: Ability ended"), __FUNCTION__ );

	if ( HasAuthority( &ActivationInfo ) )
	{
		if ( ReviveTimerHandle.IsValid() )
		{
			ActorInfo->AvatarActor->GetWorld()->GetTimerManager().ClearTimer( ReviveTimerHandle );
		}
		
		if ( ViewportCheckTask )
		{
			ViewportCheckTask->EndTask();	
		}
		
		ActorInfo->AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate( UNAAttributeSet::GetHealthAttribute() ).Remove( ReviveCancelHandle );
		
		UAbilitySystemComponent* HelperASC = ActorInfo->AbilitySystemComponent.Get();
		FGameplayTagContainer TagToRemove;
		TagToRemove.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.Reviving" ) );
		TagToRemove.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.Helping" ) );
		
		const int32 HelperRemoved = HelperASC->RemoveActiveEffectsWithAppliedTags( TagToRemove );
		UE_LOG(LogAbilitySystemComponent, Log, TEXT("%hs: Helper %d effects removed"), __FUNCTION__, HelperRemoved );
		
		if ( RevivingTarget.IsValid() )
		{
			UAbilitySystemComponent* RevivingASC = RevivingTarget->GetAbilitySystemComponent();
			const int32 RevivingRemoved = RevivingASC->RemoveActiveEffectsWithAppliedTags( TagToRemove );
			UE_LOG(LogAbilitySystemComponent, Log, TEXT("%hs: Reviving %d effects removed"), __FUNCTION__, RevivingRemoved );
			RevivingTarget->GetComponentByClass<UNAVitalCheckComponent>()->OnCharacterStateChanged.Remove( RevivingTargetHandle );
		}
	}
}

bool UNAGA_Revive::CommitAbility( const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                  const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags )
{
	bool bResult = Super::CommitAbility( Handle, ActorInfo, ActivationInfo, OptionalRelevantTags );
	const UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
	bResult &= !AbilitySystemComponent->HasMatchingGameplayTag( FGameplayTag::RequestGameplayTag( "Player.Status.Helping" ) );
	bResult &= !AbilitySystemComponent->HasMatchingGameplayTag( FGameplayTag::RequestGameplayTag( "Player.Status.Dead" ) );
	return bResult;
}

void UNAGA_Revive::InputReleased( const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo )
{
	Super::InputReleased( Handle, ActorInfo, ActivationInfo );
	CancelAbility( Handle, ActorInfo, ActivationInfo, true );
}