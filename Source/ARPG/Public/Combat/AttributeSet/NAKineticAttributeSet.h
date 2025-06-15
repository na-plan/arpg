// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "ARPG/ARPG.h"
#include "NAKineticAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNAKineticAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

	// 버틸 수 있는 시간
	UPROPERTY(VisibleAnywhere)
	FGameplayAttributeData AP;

	// 최대 버틸 수 있는 시간
	UPROPERTY(VisibleAnywhere)
	FGameplayAttributeData MaxAP;

	// 잡을 수 있는 거리
	UPROPERTY(VisibleAnywhere)
	FGameplayAttributeData Range;

	// 잡고 버틸 수 있는 최소 거리
	UPROPERTY(VisibleAnywhere)
	FGameplayAttributeData MinHoldRange;

	// 잡고 버틸 수 있는 최대 거리
	UPROPERTY(VisibleAnywhere)
	FGameplayAttributeData MaxHoldRange;

	// 밀어냈을 때의 힘
	UPROPERTY(VisibleAnywhere)
	FGameplayAttributeData Force;

public:
	ATTRIBUTE_ACCESSORS( UNAKineticAttributeSet, AP );
	ATTRIBUTE_ACCESSORS( UNAKineticAttributeSet, MaxAP );
	ATTRIBUTE_ACCESSORS( UNAKineticAttributeSet, Range );
	ATTRIBUTE_ACCESSORS( UNAKineticAttributeSet, MinHoldRange );
	ATTRIBUTE_ACCESSORS( UNAKineticAttributeSet, MaxHoldRange );
	ATTRIBUTE_ACCESSORS( UNAKineticAttributeSet, Force );

protected:
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
