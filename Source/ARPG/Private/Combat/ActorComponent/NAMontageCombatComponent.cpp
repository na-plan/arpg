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
	bool bResult = Super::IsAbleToAttack();

	// 캐릭터가 몽타주를 실행중이지 않을 경우
	// todo: 캐릭터가 공격중이지 않은 경우로?
	if ( bResult )
	{
		if ( const TScriptInterface<IAbilitySystemInterface>& Interface = GetAttacker() )
		{
			bResult &= Interface->GetAbilitySystemComponent()->GetCurrentMontage() == nullptr;
		}
	}
	
	return bResult;
}

// Called every frame
void UNAMontageCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

