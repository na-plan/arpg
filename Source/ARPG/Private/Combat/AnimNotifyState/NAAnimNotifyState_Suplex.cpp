// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/AnimNotifyState/NAAnimNotifyState_Suplex.h"

#include "AbilitySystemComponent.h"
#include "HP/GameplayEffect/NAGE_Damage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "NACharacter.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "Ability/GameplayAbility/NAGA_Melee.h"
#include "Ability/GameplayAbility/NAGA_Suplex.h"


void UNAAnimNotifyState_Suplex::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
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

	}
}

void UNAAnimNotifyState_Suplex::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp->GetWorld()->IsGameWorld())
	{
		/* 공격 판정 apply */
		
		//재사용성을 위해 clear 및 empty
		if (MeshComp->GetOwner()->HasAuthority())
		{
			SpecHandle.Clear();
			ContextHandle.Clear();
			AppliedActors.Empty();
		}

	}

}

void UNAAnimNotifyState_Suplex::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);


	// 충돌 처리는 서버의 책임
	if (MeshComp->GetOwner()->HasAuthority())
	{
		if (FrameDeltaTime)
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
							//AppiedActor
							AppliedActors.Add(OverlapResult.GetActor());
						}
					}
				}
				//검사 목록에서 NACharacter 검출 -> 이후 근접 공격 어빌리티 사용을 하는지 확인후 
				for (AActor* CheckActor : AppliedActors)
				{
					//Player Cast로 Playcheck 
					if (ANACharacter* Player = Cast<ANACharacter>(CheckActor))
					{
						UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent();
						// 공격을 하는게 combatcomponent네? 얘를 가지고 와서 해야하나?
						UNAMontageCombatComponent* PlayerCombatComponent = Player->FindComponentByClass<UNAMontageCombatComponent>();
						float ParryAngle = FVector::DotProduct(Player->GetActorForwardVector(), MeshComp->GetOwner()->GetActorForwardVector());

						//	상대 공격에 맞지 않아도 공격한 상태면 패링이 되는 문제가 있음 -> 이건 어떻게 해야할까?...
						//	생대 mesh와 owner mesh의 각도를 구해서 가져온뒤에 일정 각도 이하로 설정 ㄱ

						UAbilitySystemComponent* OwnerASC = MeshComp->GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
						UAnimInstance* AnimInstance = OwnerASC->AbilityActorInfo->GetAnimInstance();

						// 30도 이하 + 공격중 + 다른 공격이나 스킬이 사용중이 아닐경우
						if (PlayerCombatComponent->IsAttacking() && ParryAngle > 0.85 && !AnimInstance->Montage_IsPlaying(nullptr))
						{ 
							SuccessSuplex = true; 
						}
						else { SuccessSuplex = false; }

					}
				}

				
				
			}
		}
	}

	//Replicate 해야됌 /  player 는 재생 안됨
	if (MeshComp->GetWorld()->IsGameWorld())
	{
		if (UAbilitySystemComponent* OwnerASC = MeshComp->GetOwner()->FindComponentByClass<UAbilitySystemComponent>())
		{
			if (UAnimInstance* AnimInstance = OwnerASC->AbilityActorInfo->GetAnimInstance())
			{
				//suplex 성공
				if (SuccessSuplex)
				{
					for (AActor* CheckActor : AppliedActors)
					{
						if (ANACharacter* Player = Cast<ANACharacter>(CheckActor))
						{
							UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent();
							UAnimInstance* PlayerAnimInstance = PlayerASC->AbilityActorInfo->GetAnimInstance();
							//그러면 여기에서 gameability를 재생시켜버릴까?
							if (PlayerASC->GetAvatarActor()->HasAuthority())
							{
								UNAMontageCombatComponent* PlayerCombatComponent = Player->FindComponentByClass<UNAMontageCombatComponent>();

								FGameplayAbilitySpec* AbilitySpec = PlayerASC->FindAbilitySpecFromClass(UNAGA_Melee::StaticClass());
								UNAGA_Melee* NAGA_Melee = Cast<UNAGA_Melee>(AbilitySpec->Ability);								
								//PlayerAnimInstance->Montage_Stop(0.2f);
								PlayerASC->PlayMontage(NAGA_Melee, AbilitySpec->ActivationInfo, SuplexMontage, 1);								
							}
							
							//AnimInstance->Montage_Stop(0.2f);
							AnimInstance->Montage_Play(SuplexedMontage);

							FGameplayEffectSpecHandle PlayerSpecHandle = PlayerASC->MakeOutgoingSpec(UNAGE_Damage::StaticClass(), Damage, ContextHandle);
							OwnerASC->ApplyGameplayEffectSpecToTarget(*PlayerSpecHandle.Data.Get(), OwnerASC);

						}
					}
				}

			}
		}

	}
}
