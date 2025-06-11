// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Item/ItemDataStructs/NAItemBaseDataStructs.h"
#include "NARecoveryPackDataStructs.generated.h"

UENUM(BlueprintType)
enum class ERecoverableStatType : uint8
{
	RT_None			UMETA(Hidden),
	
	RT_Hp			UMETA(DisplayName = "Hp"),
	RT_Stasis		UMETA(DisplayName = "Stasis"),
	//RT_Mana		UMETA(DisplayName = "Mana"),
	//RT_Shield		UMETA(DisplayName = "Shield"),
	RT_Oxygen		UMETA(DisplayName = "Oxygen"),
};

UENUM(BlueprintType)
enum class EMedPackGrade : uint8
{
	MPG_None			UMETA(Hidden),

	MPG_Small			UMETA(DisplayName = "Small"),
	MPG_Medium			UMETA(DisplayName = "Medium"),
	MPG_Large			UMETA(DisplayName = "Large"),
};


// 회복 팩: 별도의 사용 단축키 존재(인벤 안켜고 단축키로 바로 사용 가능)
USTRUCT()
struct ARPG_API FNARecoveryPackDataStructs : public FNAItemBaseTableRow
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Recovery")
	ERecoverableStatType RecoveryTargetStat = ERecoverableStatType::RT_Hp;

	UPROPERTY(EditAnywhere, Category = "Recovery",
		meta=(EditCondition="RecoveryTargetStat==ERecoverableStatType::RT_Hp", EditConditionHides))
	EMedPackGrade MedPackGrade = EMedPackGrade::MPG_Small;

	// 회복 수치. 절댓값 기준
	UPROPERTY(EditAnywhere, Category = "Recovery", meta = (ClampMin = "0"))
	float RecoveryAmount = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Recovery", meta = (ClampMin = "0"))
	float RecoveryFactor = 0.f;

	// 비율 기반 회복인지 여부 (true면 최대치의 %로 회복)
	UPROPERTY(EditAnywhere, Category = "Recovery")
	bool bIsPercentRecovery = false;

	// 회복이 즉시 발생하는지 (true) / 지속 회복인지(false)
	UPROPERTY(EditAnywhere, Category = "Recovery")
	bool bIsInstantRecovery = true;

	// 초당 회복량. bIsInstantRecovery가 false일 때 사용
	UPROPERTY(EditAnywhere, Category = "Recovery", meta = (EditCondition = "!bIsInstantRecovery"))
	float RecoveryRate = 0.0f;

	// 지속 시간. 즉시 회복이 아닐 경우에만 의미 있음
	UPROPERTY(EditAnywhere, Category = "Recovery", meta = (EditCondition = "!bIsInstantRecovery"))
	float RecoveryDuration = 0.0f;

	// 상태이상 해제 여부. 예: 중독, 화상 등
	// 상태이상 필요하면 쓰기
	// UPROPERTY(EditAnywhere, Category = "Effect")
	// bool bRemovesNegativeEffects = false;

	// 아이템 사용 후 쿨타임
	// 쿨타임 필요할까??
	// UPROPERTY(EditAnywhere, Category = "Usage")
	// float CooldownAfterUse = 0.0f;

	// 이동 중 사용 가능 여부
	// 필요함??
	// UPROPERTY(EditAnywhere, Category = "Usage")
	// bool bCanBeUsedWhileMoving = true;
};
