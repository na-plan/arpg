// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/AnimNotifyState/NAAnimNotifyState_ParryAreaTest.h"

#include "AbilitySystemComponent.h"
#include "Ability/AttributeSet/NAAttributeSet.h"
#include "HP/GameplayEffect/NAGE_Damage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "NACharacter.h"

void UNAAnimNotifyState_ParryAreaTest::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);



	AActor* OwnerActor = MeshComp->GetOwner();

#if WITH_EDITOR
	if (GIsEditor && OwnerActor && OwnerActor->GetWorld() != GWorld) { return; }
#endif

	if (MeshComp->GetWorld()->IsGameWorld())
	{
		if (MeshComp->GetOwner()->HasAuthority())
		{
			//const TScriptInterface<IAbilitySystemInterface>& SourceInterface = MeshComp->GetOwner();

			//if (!SourceInterface)
			//{
			//	// GAS가 없는 객체로부터 시도됨
			//	check(false);
			//	return;
			//}
			if (UAbilitySystemComponent* OwnerASC = OwnerActor->FindComponentByClass<UAbilitySystemComponent>())
			{
				const FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
				ContextHandle = OwnerASC->MakeEffectContext();
				ContextHandle.AddOrigin(SocketLocation);
				ContextHandle.AddInstigator(MeshComp->GetOwner()->GetInstigatorController(), MeshComp->GetOwner());
				ContextHandle.SetAbility(OwnerASC->GetAnimatingAbility());
				ContextHandle.AddSourceObject(this);
				SpecHandle = OwnerASC->MakeOutgoingSpec(UNAGE_Damage::StaticClass(), 1.f, ContextHandle);

			}
		}
		// 서버와 클라이언트 간 플레이어 컨트롤러 설정 동기화
		if (MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>())
		{
			MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>()->bUseControllerDesiredRotation = false;
			MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>()->StopMovementImmediately();
			MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>()->DisableMovement();
		}
		// monster는 aicontroller가 서버에서 만들어 지기 때문에 동기화 과정은 필요 없을거 같음
		//else if (MeshComp->GetOwner()->GetComponentByClass<UPawnMovementComponent>())
		//{
		//	MeshComp->GetOwner()->GetComponentByClass<UPawnMovementComponent>()->bUseControllerDesiredRotation = false;
		//	MeshComp->GetOwner()->GetComponentByClass<UPawnMovementComponent>()->StopMovementImmediately();
		//	MeshComp->GetOwner()->GetComponentByClass<UPawnMovementComponent>()->DisableMovement();
		//}

	}

}

void UNAAnimNotifyState_ParryAreaTest::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	/*End에서 사용할경우*/
	if (UAbilitySystemComponent* OwnerASC = MeshComp->GetOwner()->FindComponentByClass<UAbilitySystemComponent>()) 
	{
		UAnimInstance* AnimInstance = OwnerASC->AbilityActorInfo->GetAnimInstance();
		if (Check)
		{
			AnimInstance->Montage_Stop(0.2f);
			AnimInstance->Montage_Play(StunMontage);

			
		}

	}
}

void UNAAnimNotifyState_ParryAreaTest::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);


	// 충돌 처리는 서버의 책임
	if (MeshComp->GetOwner()->HasAuthority())
	{
		// 충돌 확인 지연	-> 어차피 텀은 적은데 그냥 tick에서 돌리는게 맞지 않을까? 
		OverlapElapsed += FrameDeltaTime;
		//interval time 넘어가면 충돌 확인 계속 하도록 하는거
		if (OverlapElapsed >= OverlapInterval)
		{
			const FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
			TArray<FOverlapResult> OverlapResults;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(MeshComp->GetOwner()); // 시전자 제외
			const bool bOverlap = MeshComp->GetWorld()->OverlapMultiByChannel
			(
				OverlapResults,
				SocketLocation,
				FQuat::Identity,
				ECC_Pawn,
				FCollisionShape::MakeSphere(SphereRadius),
				QueryParams
			);

#if WITH_EDITOR || UE_BUILD_DEBUG
			DrawDebugSphere
			(
				MeshComp->GetWorld(),
				SocketLocation,
				SphereRadius,
				8,
				bOverlap || !OverlapResults.IsEmpty() ? FColor::Green : FColor::Red
			);
#endif
			//충돌된게 게임월드일 경우
			if (!OverlapResults.IsEmpty() && MeshComp->GetWorld()->IsGameWorld())
			{
				const TScriptInterface<IAbilitySystemInterface>& SourceInterface = MeshComp->GetOwner();

				if (!SourceInterface)
				{
					// GAS가 없는 객체로부터 시도됨
					check(false);
					return;
				}
				//AppliedActors 에 새로운 대상이 추가되면 검사 목록에 추가
				for (const FOverlapResult& OverlapResult : OverlapResults)
				{
					if (const TScriptInterface<IAbilitySystemInterface>& TargetInterface = OverlapResult.GetActor())
					{
						if (!AppliedActors.Contains(OverlapResult.GetActor()))
						{
							UE_LOG(LogTemp, Log, TEXT("[%hs]: Found target %s"), __FUNCTION__, *OverlapResult.GetActor()->GetName());
							SourceInterface->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget
							(
								*SpecHandle.Data.Get(),
								TargetInterface->GetAbilitySystemComponent()
							);

							AppliedActors.Add(OverlapResult.GetActor());
						}
					}
				}
				//검사 목록에서 NACharacter 검출 -> 이후 근접 공격 어빌리티 사용을 하는지 확인후 
				for (AActor* Test : AppliedActors)
				{
					//if(AppliedActors)
				}
				

			}

			OverlapElapsed = 0.f;
		}

	}




}
