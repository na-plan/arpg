// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "ARPG/ARPG.h"
#include "MonsterAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UMonsterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	// 체력
	UPROPERTY(VisibleAnywhere, Replicated)
	FGameplayAttributeData Health;

	// 공격력
	UPROPERTY(VisibleAnywhere, Replicated)
	FGameplayAttributeData Power;

	// note: 만약 새로운 속성을 추가할 경우 아래의 매크로를 같이 추가해주어야 함!
	ATTRIBUTE_ACCESSORS(UMonsterAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(UMonsterAttributeSet, Power);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
};
