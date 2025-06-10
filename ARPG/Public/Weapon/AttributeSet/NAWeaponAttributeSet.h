// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "ARPG/ARPG.h"
#include "NAWeaponAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNAWeaponAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	// 체력
	UPROPERTY(VisibleAnywhere, Replicated)
	FGameplayAttributeData TotalAmmoCount;

	// 스테미나
	UPROPERTY(VisibleAnywhere, Replicated)
	FGameplayAttributeData LoadedAmmoCount;

	// note: 만약 새로운 속성을 추가할 경우 아래의 매크로를 같이 추가해주어야 함!
	ATTRIBUTE_ACCESSORS(UNAWeaponAttributeSet, TotalAmmoCount);
	ATTRIBUTE_ACCESSORS(UNAWeaponAttributeSet, LoadedAmmoCount);

protected:

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
