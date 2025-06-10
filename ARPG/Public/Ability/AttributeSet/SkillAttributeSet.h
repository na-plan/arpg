// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ARPG/ARPG.h"
#include "AbilitySystemComponent.h"
#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "SkillAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API USkillAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	//Skill Attribute Table 만들기
	
public:
	// Cooltime
	UPROPERTY(VisibleAnywhere, Replicated)
	FGameplayAttributeData CoolTime;
	// Damage
	UPROPERTY(VisibleAnywhere, Replicated)
	FGameplayAttributeData Damage;
	// mana 있으면 사용
	UPROPERTY(VisibleAnywhere, Replicated)
	FGameplayAttributeData Cost;

	ATTRIBUTE_ACCESSORS(USkillAttributeSet , CoolTime);
	ATTRIBUTE_ACCESSORS(USkillAttributeSet , Damage);
	ATTRIBUTE_ACCESSORS(USkillAttributeSet , Cost);
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	
};
