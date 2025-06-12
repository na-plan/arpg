// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "ARPG/ARPG.h"
#include "NAAttributeSet.generated.h"

/**
 * 일반적으로 객체에 필요로한 속성
 */
UCLASS()
class ARPG_API UNAAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	// 체력
	UPROPERTY(VisibleAnywhere, Replicated)
	FGameplayAttributeData Health;

	// 최대 체력
	UPROPERTY(VisibleAnywhere, Replicated)
	FGameplayAttributeData MaxHealth;

	// 스테미나
	UPROPERTY(VisibleAnywhere, Replicated)
	FGameplayAttributeData AP;

	// 이동속력
	UPROPERTY(VisibleAnywhere, Replicated)
	FGameplayAttributeData MovementSpeed;

	// note: 만약 새로운 속성을 추가할 경우 아래의 매크로를 같이 추가해주어야 함!
	ATTRIBUTE_ACCESSORS(UNAAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(UNAAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UNAAttributeSet, AP);
	ATTRIBUTE_ACCESSORS(UNAAttributeSet, MovementSpeed);
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data) override;
	
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
};
