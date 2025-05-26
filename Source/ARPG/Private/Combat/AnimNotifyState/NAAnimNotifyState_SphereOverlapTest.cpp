// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/AnimNotifyState/NAAnimNotifyState_SphereOverlapTest.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Engine/OverlapResult.h"
#include "HP/GameplayEffect/NAGE_Damage.h"

void UNAAnimNotifyState_SphereOverlapTest::NotifyBegin( USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                        float TotalDuration, const FAnimNotifyEventReference& EventReference )
{
	Super::NotifyBegin( MeshComp, Animation, TotalDuration, EventReference );

	if ( MeshComp->GetWorld()->IsGameWorld() )
	{
		const TScriptInterface<IAbilitySystemInterface>& SourceInterface = MeshComp->GetOwner();

		if ( !SourceInterface )
		{
			// GAS가 없는 객체로부터 시도됨
			check( false );
			return;
		}

		const FVector SocketLocation = MeshComp->GetSocketLocation( SocketName );
	
		ContextHandle = SourceInterface->GetAbilitySystemComponent()->MakeEffectContext();
		ContextHandle.AddOrigin( SocketLocation );
		ContextHandle.AddInstigator( MeshComp->GetOwner()->GetInstigatorController(), MeshComp->GetOwner() );
		ContextHandle.SetAbility( SourceInterface->GetAbilitySystemComponent()->GetAnimatingAbility() );
		ContextHandle.AddSourceObject( this );

		SpecHandle = SourceInterface->GetAbilitySystemComponent()->MakeOutgoingSpec( UNAGE_Damage::StaticClass(), 1.f, ContextHandle );
	}
}

void UNAAnimNotifyState_SphereOverlapTest::NotifyEnd( USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference )
{
	Super::NotifyEnd( MeshComp, Animation, EventReference );

	if ( MeshComp->GetWorld()->IsGameWorld() )
	{
		SpecHandle.Clear();
		ContextHandle.Clear();
		AppliedActors.Empty();
	}
}

void UNAAnimNotifyState_SphereOverlapTest::NotifyTick( USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                       float FrameDeltaTime, const FAnimNotifyEventReference& EventReference )
{
	Super::NotifyTick( MeshComp, Animation, FrameDeltaTime, EventReference );

	// 충돌 확인 지연
	OverlapElapsed += FrameDeltaTime;

	if (OverlapElapsed >= OverlapInterval)
	{
		const FVector SocketLocation = MeshComp->GetSocketLocation( SocketName );
		TArray<FOverlapResult> OverlapResults;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor( MeshComp->GetOwner() ); // 시전자 제외
		const bool bOverlap = MeshComp->GetWorld()->OverlapMultiByChannel
		(
			OverlapResults,
			SocketLocation,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere( SphereRadius ),
			QueryParams
		);

#if WITH_EDITOR || UE_BUILD_DEBUG
		DrawDebugSphere( MeshComp->GetWorld(), SocketLocation, SphereRadius, 16, bOverlap ? FColor::Green : FColor::Red );
#endif

		if ( !OverlapResults.IsEmpty() && MeshComp->GetWorld()->IsGameWorld() )
		{
			const TScriptInterface<IAbilitySystemInterface>& SourceInterface = MeshComp->GetOwner();

			if ( !SourceInterface )
			{
				// GAS가 없는 객체로부터 시도됨
				check( false );
				return;
			}
			
			for (const FOverlapResult& OverlapResult : OverlapResults)
			{
				if ( const TScriptInterface<IAbilitySystemInterface>& TargetInterface = OverlapResult.GetActor() )
				{
					if ( !AppliedActors.Contains( OverlapResult.GetActor() ) )
					{
						UE_LOG(LogTemp, Log, TEXT( "[%hs]: Found target %s" ), __FUNCTION__, *OverlapResult.GetActor()->GetName() );
						SourceInterface->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget
						(
							*SpecHandle.Data.Get(),
							TargetInterface->GetAbilitySystemComponent()
						);
						
						AppliedActors.Add( OverlapResult.GetActor() );
					}
				}	
			}
		}		

		OverlapElapsed = 0.f;
	}
}
