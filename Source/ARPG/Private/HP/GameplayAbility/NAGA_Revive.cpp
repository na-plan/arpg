// Fill out your copyright notice in the Description page of Project Settings.


#include "HP/GameplayAbility/NAGA_Revive.h"

#include "AbilitySystemComponent.h"
#include "NACharacter.h"
#include "Ability/AttributeSet/NAAttributeSet.h"
#include "Ability/GameAbilityTask/NAAT_WaitPlayerViewport.h"
#include "HP/GameplayEffect/NAGE_Damage.h"

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

		EndAbility( GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false );
	}
}

void UNAGA_Revive::OnHitWhileRevive( const FOnAttributeChangeData& OnAttributeChangeData )
{
	// 데미지를 입거나 쓰러지면 부활을 취소
	if ( OnAttributeChangeData.NewValue < OnAttributeChangeData.OldValue ||
		 OnAttributeChangeData.NewValue <= 0.f )
	{
		CancelAbility( GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true );
	}
}

void UNAGA_Revive::ActivateAbility( const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData )
{
	Super::ActivateAbility( Handle, ActorInfo, ActivationInfo, TriggerEventData );

	if (HasAuthority( &ActivationInfo ))
	{
		if (!CommitAbility( Handle, ActorInfo, ActivationInfo ))
		{
			EndAbility( Handle, ActorInfo, ActivationInfo, true, true );
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

					RevivingTarget = Character;
					ActorInfo->AbilitySystemComponent->AddReplicatedLooseGameplayTag( FGameplayTag::RequestGameplayTag( "Player.Status.Reviving" ) );

					// 부활시키는 캐릭터가 화면 밖으로 나가지 않도록
					ViewportCheckTask = UNAAT_WaitPlayerViewport::WaitPlayerViewport( this, "WaitPlayerViewport", ActorInfo->AbilitySystemComponent.Get(), Character );
					ViewportCheckTask->ReadyForActivation();
					
					ActorInfo->AvatarActor->GetWorld()->GetTimerManager().SetTimer( ReviveTimer, this, &UNAGA_Revive::OnReviveSucceeded, 5.f, false );
					ReviveCancelHandle = ActorInfo->AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate( UNAAttributeSet::GetHealthAttribute() ).AddUObject( this, &UNAGA_Revive::OnHitWhileRevive );
					break;
				}
			}
		}
	}
}

void UNAGA_Revive::CancelAbility( const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility )
{
	Super::CancelAbility( Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility );
}

void UNAGA_Revive::EndAbility( const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled )
{
	Super::EndAbility( Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled );

	if ( HasAuthority( &ActivationInfo ) )
	{
		ViewportCheckTask->EndTask();
		
		if ( ReviveTimer.IsValid() )
		{
			ActorInfo->AvatarActor->GetWorld()->GetTimerManager().ClearTimer( ReviveTimer );
		}
		
		ActorInfo->AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate( UNAAttributeSet::GetHealthAttribute() ).Remove( ReviveCancelHandle );
		
		if ( ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag( FGameplayTag::RequestGameplayTag( "Player.Status.Reviving" ) ) )
		{
			ActorInfo->AbilitySystemComponent->RemoveReplicatedLooseGameplayTag( FGameplayTag::RequestGameplayTag( "Player.Status.Reviving" ) );
		}
		else
		{
			// 분명히 전에 태그가 붙었어야 함...
			check( false );
		}
	}
}

bool UNAGA_Revive::CommitAbility( const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                  const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags )
{
	bool bResult = Super::CommitAbility( Handle, ActorInfo, ActivationInfo, OptionalRelevantTags );
	bResult &= !ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag( FGameplayTag::RequestGameplayTag( "Player.Status.Reviving" ) );
	bResult &= !ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag( FGameplayTag::RequestGameplayTag( "Player.Status.Alive" ) );
	return bResult;
}
