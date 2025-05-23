// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/GE_ApplyActor.h"

#include "Components/SphereComponent.h"
#include "AbilitySystemComponent.h"
#include "NACharacter.h"
#include "Monster/Pawn/MonsterBase.h"
#include "HP/GameplayEffect/NAGE_Damage.h"
// Sets default values
AGE_ApplyActor::AGE_ApplyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	RootComponent = SphereCollision;
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

}

void AGE_ApplyActor::SphereCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 자기 자신이면 return
	if (GetInstigator() == OtherActor)
	{
		return;
	}
	

	//Monster가 사용시
	if (AMonsterBase* Owner = Cast<AMonsterBase>(GetInstigator()->GetOwner()))
	{
		if (ANACharacter* Target = Cast<ANACharacter>(OtherActor))
		{
			if (Target->GetAbilitySystemComponent())
			{
				for (auto Effect : EffectsToApply)
				{
					// 자신이 Target에게Damage
					FGameplayEffectContextHandle EffectContext = Target->GetAbilitySystemComponent()->MakeEffectContext();
					EffectContext.AddInstigator(GetInstigator(), Target);

					// Gameplay Effect CDO, 레벨?, ASC에서 부여받은 Effect Context로 적용할 효과에 대한 설명을 생성
					const FGameplayEffectSpecHandle DamageEffectSpec = Target->GetAbilitySystemComponent()->MakeOutgoingSpec(UNAGE_Damage::StaticClass(), 1, EffectContext);

					// damage 미확인중 notify 만들고 데미지 안입힐시 위 주석 해제 ㄱㄱ
					{
						// 일단 데미지만 주면 되기 때문에 자기 자신이데미지 주는 형식으로....
						const auto& Handle = Target->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*DamageEffectSpec.Data.Get());

						// Target 에게 공격
						const auto& Handle = Owner->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*DamageEffectSpec.Data.Get(), Target->GetAbilitySystemComponent());
						check(Handle.WasSuccessfullyApplied());
					}
				}

			}

		}
	}
	//Player 가 사용시
	else if (ANACharacter* Owner = Cast<ANACharacter>(GetInstigator()->GetOwner()))
	{
		if (AMonsterBase* Target = Cast<AMonsterBase>(OtherActor))
		{
			if (Target->GetAbilitySystemComponent())
			{
				for (auto Effect : EffectsToApply)
				{
					// 자신이 Target에게Damage
					FGameplayEffectContextHandle EffectContext = Target->GetAbilitySystemComponent()->MakeEffectContext();
					EffectContext.AddInstigator(GetInstigator(), Target);

					// Gameplay Effect CDO, 레벨?, ASC에서 부여받은 Effect Context로 적용할 효과에 대한 설명을 생성
					// UNAGE_Damage에도 슬슬 데미지값을 넣어주는게 맞지 않을까?
					const FGameplayEffectSpecHandle DamageEffectSpec = Target->GetAbilitySystemComponent()->MakeOutgoingSpec(UNAGE_Damage::StaticClass(), 1, EffectContext);

					// damage 미확인중 notify 만들고 데미지 안입힐시 위 주석 해제 ㄱㄱ
					{
						// 일단 데미지만 주면 되기 때문에 자기 자신이데미지 주는 형식으로....
						//const auto& Handle = Target->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*DamageEffectSpec.Data.Get());

						// Target 에게 공격
						const auto& Handle = Owner->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*DamageEffectSpec.Data.Get(), Target->GetAbilitySystemComponent());
						check(Handle.WasSuccessfullyApplied());
					}

				}

			}
		}
	}

	
}

// Called when the game starts or when spawned
void AGE_ApplyActor::BeginPlay()
{
	Super::BeginPlay();

	//
	SetLifeSpan(LifeTime);

	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AGE_ApplyActor::SphereCollisionBeginOverlap);
}

// Called every frame
void AGE_ApplyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

