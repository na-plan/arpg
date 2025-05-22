// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/ActorComponent/NAMontageCombatComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UNAMontageCombatComponent::UNAMontageCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UNAMontageCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

bool UNAMontageCombatComponent::IsAbleToAttack()
{
	bool bResult = true;
	
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (const TScriptInterface<IAbilitySystemInterface>& Interface = Pawn)
		{
			bResult &= Interface->GetAbilitySystemComponent()->GetCurrentMontage() == nullptr;
		}
	}
	
	return bResult;
}

void UNAMontageCombatComponent::OnAttack_Implementation()
{
	// note: 몽타주 재생 및 전파는 GAS의 Ability를 통해 수행!
	
	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttackSound, GetOwner()->GetActorLocation());

		// 서버 구동일 경우에는 사운드 재생을 전파
		if (GetNetMode() != NM_Standalone)
		{
			Multi_PlaySound();
		}
	}

	if (const TScriptInterface<IAbilitySystemInterface>& Interface = GetOwner())
	{
		CachedAttackMontage = Interface->GetAbilitySystemComponent()->GetCurrentMontage();
		CachedAttackMontagePlayRate = Interface->GetAbilitySystemComponent()->AbilityActorInfo->GetAnimInstance()->Montage_GetPlayRate(CachedAttackMontage);
	}
}

void UNAMontageCombatComponent::Multi_PlaySound_Implementation() const
{
	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttackSound, GetOwner()->GetActorLocation());
	}
}

float UNAMontageCombatComponent::GetNextAttackTime()
{
	return CachedAttackMontage->GetPlayLength() * CachedAttackMontagePlayRate;
}

// Called every frame
void UNAMontageCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

