// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NAAbilityActor.generated.h"

class UGameplayAbility;
// This class does not need to be modified.
UINTERFACE()
class UNAAbilityActor : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPG_API INAAbilityActor
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual TSubclassOf<UGameplayAbility> GetAbility() const = 0;
};
