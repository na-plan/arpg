// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NAUpgradeNodeDataStructs.generated.h"


USTRUCT(BlueprintType)
struct FNASuitUpgradeStatusData
{
	GENERATED_BODY()

// RIG: 슈트 기본 능력치 ////////////////////////
	// 체력
	UPROPERTY()
	float Health = 0.f;

	// 산소
	UPROPERTY()
	float Oxygen = 0.f;

	// 키네시스 투척 데미지
	UPROPERTY()
	float KinesisThrowDamage = 0.f;

// 키네시스 모듈 ////////////////////////////////	

	UPROPERTY()
	uint8 bActivateKinesisModule : 1 = false;
	
	// 키네시스 그랩 사거리
	UPROPERTY()
	float KinesisGrabRange = 0.f;

// 스테이시스 모듈 //////////////////////////////

	UPROPERTY()
	uint8 bActivateStasisModule : 1 = false;

	// 스테이시스 지속 시간
	UPROPERTY()
	float StasisDuration = 0.f;

	// 스테이시스 에너지
	UPROPERTY()
	float StasisEnergy = 0.f;

	// 스테이시스 존 (범위 증가)
	UPROPERTY()
	float StasisZone = 0.f;
};

// 이 구조체 상속해서 무기 전용 강화 데이터 구조체 만들기
USTRUCT(BlueprintType)
struct FNAWeaponUpgradeStatusDataBase
{
	GENERATED_BODY()

	// 공격력
	UPROPERTY()
	float Damage = 0.f;

	// 용량
	UPROPERTY()
	float Capacity = 0.f;

	// 장전 시간
	UPROPERTY()
	float ReloadTime = 0.f;
};

USTRUCT(BlueprintType)
struct FNAPlasmaCutterUpgradeStatusData : public FNAWeaponUpgradeStatusDataBase
{
	GENERATED_BODY()

	// 연사력
	UPROPERTY()
	float RateOfFire = 0.f;

	// 축열기: 공격 시간에 따른 추가 데미지 적용
	UPROPERTY()
	uint8 Special01_HeatAccumulator : 1 = false;

	// 탄창 랙: 탄약 용량 크게 증가
	UPROPERTY()
	uint8 Special02_CartridgeRack : 1 = false;

	// 무거운 날: 근접 공격으로 엎드린 적 처치 가능
	UPROPERTY()
	uint8 Special03_WeightedBlades : 1 = false;
};

USTRUCT(BlueprintType)
struct FNAPulseRifleUpgradeStatusData : public FNAWeaponUpgradeStatusDataBase
{
	GENERATED_BODY()

	// 키네틱 오토로더: 연사력 크게 증가
	UPROPERTY()
	uint8 Special01_KineticAutoloader : 1 = false;

	// 커스텀 탄창: 탄약 용량 크게 증가
	UPROPERTY()
	uint8 Special02_PCSICustomMagazine : 1 = false;

	// 고성능 수류탄: 수류탄 폭발 반경 크게 증가
	UPROPERTY()
	uint8 Special03_HighYieldGrenades : 1 = false;
};