﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPG/Public/NAPlayerState.h"

#include "Net/UnrealNetwork.h"

void ANAPlayerState::SetHealth(const int32 NewHealth)
{
	Health = FMath::Clamp(NewHealth, 0, MaxHealth);
}

void ANAPlayerState::IncreaseHealth(int32 Increment)
{
	if (Increment < 0)
	{
		return DecreaseHealth(-Increment);
	}
	
	if (Increment > 0 && Health > std::numeric_limits<int32>::max() - Increment)
	{
		Health = MaxHealth;
	}
	else if (Health + Increment >= MaxHealth)
	{
		Health = MaxHealth;
	}
	else
	{
		Health += Increment;
	}
}

void ANAPlayerState::DecreaseHealth(const int32 Decrement)
{
	if (Decrement < 0)
	{
		return IncreaseHealth(-Decrement);
	}

	if (Decrement > 0 && Health < std::numeric_limits<int32>::min() + Decrement)
	{
		Health = MaxHealth;
	}
	else if (Health - Decrement <= 0)
	{
		Health = 0;
	}
	else
	{
		Health -= Decrement;
	}
}

bool ANAPlayerState::IsAlive() const
{
	return Health > 0;
}

void ANAPlayerState::OnCharacterTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                              AController* InstigatedBy, AActor* DamageCauser)
{
	const int32 BeforeHealth = Health;
	DecreaseHealth(static_cast<int32>(Damage));
	OnHealthChanged.Broadcast(BeforeHealth, GetHealth());
}

void ANAPlayerState::BeginPlay()
{
	Super::BeginPlay();

	// == 테스트 코드 ==
	MaxHealth = 100;
	// ===============
	
	Health = MaxHealth;
}

void ANAPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANAPlayerState, Health)
}
