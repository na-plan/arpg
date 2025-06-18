// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NAAbilityGameInstanceSubsystem.generated.h"

class IAbilitySystemInterface;
/**
 * 
 */
UCLASS()
class ARPG_API UNAAbilityGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	// 모든 객체(캐릭터, NPC, 몹 등등..)에 대한 Attributes를 담고 있는 테이블
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	UDataTable* AttributesDataTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	UDataTable* KineticDataTable;

public:
	// 모든 객체에 대한 Attributes를 담고 있는 테이블을 반환
	UFUNCTION(BlueprintCallable)
	UDataTable* GetCharacterAttributesDataTable() const { return AttributesDataTable; }

	UFUNCTION(BlueprintCallable)
	UDataTable* GetKineticAttributesDataTable() const { return KineticDataTable; }

	// 특정 객체에 대한 Attributes를 담고 있는 테이블을 반환
	UFUNCTION(BlueprintCallable)
	UDataTable* GetAttributeDataTable(const FName& InRowName) const;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
};
