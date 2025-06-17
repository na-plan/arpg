// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "ARPG/ARPG.h"
#include "NAAttributeSet.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams( FOnNAAttributeChanged, float, float );

/**
 * 일반적으로 객체에 필요로한 속성
 */
UCLASS()
class ARPG_API UNAAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	// 체력
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_Health)
	FGameplayAttributeData Health;

	// 최대 체력
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;

	// 스테미나
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_AP)
	FGameplayAttributeData AP;

	// 이동속력
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_MovementSpeed)
	FGameplayAttributeData MovementSpeed;

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& Old) const
	{
		OnHealthChanged.Broadcast( Old.GetCurrentValue(), GetHealth() );
	}

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& Old) const
	{
		OnMaxHealthChanged.Broadcast( Old.GetCurrentValue(), GetMaxHealth() );
	}

	UFUNCTION()
	void OnRep_AP(const FGameplayAttributeData& Old) const
	{
		OnAPChanged.Broadcast( Old.GetCurrentValue(), GetAP() );
	}

	UFUNCTION()
	void OnRep_MovementSpeed(const FGameplayAttributeData& Old) const
	{
		OnMaxHealthChanged.Broadcast( Old.GetCurrentValue(), GetMovementSpeed() );
	}
	
public:
	mutable FOnNAAttributeChanged OnHealthChanged;
	mutable FOnNAAttributeChanged OnMaxHealthChanged;
	mutable FOnNAAttributeChanged OnAPChanged;
	mutable FOnNAAttributeChanged OnMovementSpeedChanged;
	
	// note: 만약 새로운 속성을 추가할 경우 아래의 매크로를 같이 추가해주어야 함!
	ATTRIBUTE_ACCESSORS(UNAAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(UNAAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UNAAttributeSet, AP);
	ATTRIBUTE_ACCESSORS(UNAAttributeSet, MovementSpeed);
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	void MonsterFindInstigator(FGameplayEffectModCallbackData Data);

};
