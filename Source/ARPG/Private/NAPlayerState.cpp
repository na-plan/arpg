// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPG/Public/NAPlayerState.h"

#include "AbilitySystemComponent.h"
#include "NACharacter.h"
#include "Ability/AttributeSet/NAAttributeSet.h"
#include "Assets/Interface/NAManagedAsset.h"
#include "Net/UnrealNetwork.h"

float ANAPlayerState::GetHealth() const
{
	// 캐릭터의 AbilitySystemComponent로부터 체력을 조회
	if (const ANACharacter* Character = Cast<ANACharacter>(GetPlayerController()->GetCharacter()))
	{
		const UNAAttributeSet* AttributeSet = Cast<UNAAttributeSet>(Character->GetAbilitySystemComponent()->GetAttributeSet(UNAAttributeSet::StaticClass()));
		return AttributeSet->GetHealth();
	}

	// todo: NPC의 경우 핸들링
	check(false);
	
	return 0.f;
}

int32 ANAPlayerState::GetMaxHealth() const
{
	// 캐릭터의 AbilitySystemComponent로부터 체력을 조회
	if (const ANACharacter* Character = Cast<ANACharacter>(GetPlayerController()->GetCharacter()))
	{
		const UNAAttributeSet* AttributeSet = Cast<UNAAttributeSet>(Character->GetAbilitySystemComponent()->GetAttributeSet(UNAAttributeSet::StaticClass()));
		return AttributeSet->Health.GetBaseValue();
	}

	// todo: NPC의 경우 핸들링
	check(false);
	
	return 0.f;
}

bool ANAPlayerState::IsAlive() const
{
	// 캐릭터의 AbilitySystemComponent로부터 체력을 조회
	if (const ANACharacter* Character = Cast<ANACharacter>(GetPlayerController()->GetCharacter()))
	{
		const UNAAttributeSet* AttributeSet = Cast<UNAAttributeSet>(Character->GetAbilitySystemComponent()->GetAttributeSet(UNAAttributeSet::StaticClass()));
		return AttributeSet->GetHealth() > 0;
	}

	// todo: NPC의 경우 핸들링
	check(false);
	
	return false;
}

void ANAPlayerState::SetPossessAssetName(const FName& AssetName)
{
	PossessAssetName = AssetName;
	UpdatePossessAssetByName();
}

void ANAPlayerState::OnRep_PossessAssetName() const
{
	UpdatePossessAssetByName();
}

void ANAPlayerState::UpdatePossessAssetByName() const
{
	if (const TScriptInterface<INAManagedAsset> Interface = GetPawn())
	{
		Interface->SetAssetName(PossessAssetName);
	}
}

void ANAPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void ANAPlayerState::PostNetInit()
{
	Super::PostNetInit();
	
	if (GetNetMode() == NM_Client)
	{
		TScriptDelegate Delegate;
		Delegate.BindUFunction(this, "UpdatePossessAssetByName");
		OnPawnSet.AddUnique(Delegate);
	}
}

void ANAPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANAPlayerState, PossessAssetName)
}
