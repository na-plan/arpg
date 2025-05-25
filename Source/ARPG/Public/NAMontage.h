// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NAMontage.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UNAMontage : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPG_API INAMontage
{
	GENERATED_BODY()
	
public:
	virtual UAnimMontage* GetAttackMontage() const = 0;
};
