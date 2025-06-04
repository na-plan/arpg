// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/AnimNotifyState/NAAnimNotifyState_Grap.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "NACharacter.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "HP/GameplayEffect/NAGE_Damage.h"

void UNAAnimNotifyState_Grap::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	// Player의 잡기와 monster의 잡기를 구분..
	 
	 
	// Monster가 사용하면 잡기 시전 애니메이션이 먼저 나가고 해당 범위 내에 플레이어가
	// 존재할 경우 플레이어도 잡기 저항(잡기 도중) 모션이 나가도록 하고
	// 이후 일정 스택 이상 공격 성공시 또는 다른 플레이어가 근접공격 성공시 떨어져 나가도록 하기



	
	// 플레이어가 사용할 경우 
	// null offset에 monster의 root고정 시켜야 됌		//bone 보니까 이건 세팅 안한거 같음.. 일정 위치로 해야 할거 같음
	// 상대 몬스터의 회전 확인(뒤쪽일것) -> 가능하도록 하기 ㄱㄱ
	// 무기에 따른 공격이니까 idle에서 해당 notify를 불러내서 사용하면 되려나?
	// 권총, ㄴ무기 = splex	//	knife =  knifeAction으로

	// idle에서 호출했다고 치고 그러면 begin 할때 구체 만들어서 몬스터의 rot 가져오고 위치는 구체 하나 박아 놓고 
	// 해당 구체에 닿으면 추가적인 ui를 보이게 한다거나 tick에서 보여주게 하고 tick중에 해당 트리거 on하면 바로 애니메이션 몽타주 재생




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

void UNAAnimNotifyState_Grap::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{

	Super::NotifyEnd(MeshComp, Animation, EventReference);


	// 만들고 나서 보니까 이거 잘하면 잡기 판정도 만들수 있을거 같은데?...
	// 심지어 대상을 내 소켓애다가 박아놓고 tick동안 움직임 고정 시키고 end 때 풀어놓으면?
	// tick동안 대상에게 특정 montage 강제 시켜놓으면 잡기도 될거 같은데????
	// ParryArea 같이 쓰면 잡기 시전중에 패링시키고 실패시 잡혀가는것도 될거 같은데?????

	if (MeshComp->GetWorld()->IsGameWorld())
	{
		/* 공격 판정 apply */
		if (UAbilitySystemComponent* OwnerASC = MeshComp->GetOwner()->FindComponentByClass<UAbilitySystemComponent>())
		{
			UAnimInstance* AnimInstance = OwnerASC->AbilityActorInfo->GetAnimInstance();
			//패링 성공시 데미지를 입지 않고 stun상태가 됩니다
			//if (SuccessGrap)
			//{
			//	AnimInstance->Montage_Stop(0.2f);
			//	AnimInstance->Montage_Play(StunMontage);
			//}
			//패링 실패시 대상에게 데미지를 주도록 합시다
			//AppliedActors 에 set되어있는 액터중 NACharacter만 가지고 와서 데미지 ㄱㄱ
		//	else
		//	{
		//		for (AActor* CheckActor : AppliedActors)
		//		{
		//			if (ANACharacter* Player = Cast<ANACharacter>(CheckActor))
		//			{
		//				const TScriptInterface<IAbilitySystemInterface>& SourceInterface = MeshComp->GetOwner();

		//				UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent();
		//				SourceInterface->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget
		//				(
		//					*SpecHandle.Data.Get(),
		//					PlayerASC
		//				);

		//			}
		//		}

		//	}

		}

		//재사용성을 위해 clear 및 empty
		if (MeshComp->GetOwner()->HasAuthority())
		{
			SpecHandle.Clear();
			ContextHandle.Clear();
			AppliedActors.Empty();
		}

	}
}

void UNAAnimNotifyState_Grap::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
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
							//AppiedActor
							AppliedActors.Add(OverlapResult.GetActor());
						}
					}
				}

				// 검사 목록에서 NACharacter 검출-> 
				for (AActor* CheckActor : AppliedActors)
				{
					//Player Cast로 Playcheck 
					if (ANACharacter* Player = Cast<ANACharacter>(CheckActor))
					{
						UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent();
						// 공격을 하는게 combatcomponent네? 얘를 가지고 와서 해야하나?
						UNAMontageCombatComponent* PlayerCombatComponent = Player->FindComponentByClass<UNAMontageCombatComponent>();
						float GrapAngle = FVector::DotProduct(Player->GetActorForwardVector(), MeshComp->GetOwner()->GetActorForwardVector());

						//	상대 공격에 맞지 않아도 공격한 상태면 패링이 되는 문제가 있음 -> 이건 어떻게 해야할까?...
						//	생대 mesh와 owner mesh의 각도를 구해서 가져온뒤에 일정 각도 이하로 설정 ㄱ
						

						// 플레이어가 공격중이 아닐경우 잡기 성공	parry도 같이넣어서 공격중이면 패링 ㄱ
						if (!PlayerCombatComponent->IsAttacking()) { SuccessGrap = true; }
						else { SuccessGrap = false; }

					}

				}


			}

			OverlapElapsed = 0.f;
		}

	}


}
