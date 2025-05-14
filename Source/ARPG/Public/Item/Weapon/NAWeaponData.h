#pragma once

#include "Item/NAItemBaseData.h"

#include "NAWeaponData.generated.h"


UENUM(BlueprintType)
enum class EWeaponRarity : uint8
{
	WR_None        UMETA(DisplayName = "None"),        // 초기화 필요

	WR_Common      UMETA(DisplayName = "Common"),      // 일반 (흰색)
	WR_Uncommon    UMETA(DisplayName = "Uncommon"),    // 고급 (녹색)
	WR_Rare        UMETA(DisplayName = "Rare"),        // 희귀 (파란색)
	WR_Epic        UMETA(DisplayName = "Epic"),        // 영웅 (보라색)
	WR_Legendary   UMETA(DisplayName = "Legendary"),   // 전설 (주황색)
	WR_Mythic      UMETA(DisplayName = "Mythic"),      // 신화 (붉은색 혹은 금색)
};

UENUM(BlueprintType)
enum class  EWeaponType : uint8
{
	WT_None        UMETA(DisplayName = "None"),		// 초기화 필요

	WT_Blunt       UMETA(DisplayName = "Blunt"),      // 둔기: 몽둥이, 망치, 철퇴 등
	WT_Blade       UMETA(DisplayName = "Blade"),      // 예기: 검, 도, 창 등 날붙이
	WT_Firearm     UMETA(DisplayName = "Firearm")     // 총기: 권총, 소총, 산탄총 등
};

USTRUCT()
struct FNAWeaponStatistics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Weapon Statistics")
	EWeaponRarity WeaponRarity = EWeaponRarity::WR_Common;

	UPROPERTY(EditAnywhere, Category = "Weapon Statistics")
	float WeaponDamage = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Weapon Statistics")
	float WeaponAttackSpeed = 0.0f;
};

class ANAProjectile;
USTRUCT()
struct FNAFirearmStatistics 
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	TSubclassOf<ANAProjectile> ProjectileClass;	// 발사체 클래스

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	int32 MaxAmmo = -1;

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	float ReloadTime = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	float FireRate = 0.0f;	// 초당 발사 수, 연사 속도

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	float BulletSpread = 0.0f;	// 총알 퍼짐 정도

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	float RecoilStrength = 0.0f;	// 반동 강도

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	float EffectiveRange = 0.0f;	// 유효 사거리

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	int32 PierceCount = -1; 	// 관통 수, 총알이 적을 몇 마리 관통할 수 있는지

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	int32 AmmoPerShot = -1; 	// 한 번 발사할 때 소비되는 탄 수

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	bool bIsAutomatic = false;	// 자동 발사 여부

	//UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	//EFireMode FireMode = EFireMode::FM_Single;	// 발사 모드 (단발, 점사, 연사 등)
};

USTRUCT()
struct ARPG_API FNAWeaponTableRow : public FNAItemBaseTableRow
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Weapon")
	EWeaponType WeaponType = EWeaponType::WT_None;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	FNAWeaponStatistics WeaponStatistics;

	UPROPERTY(EditAnywhere, Category = "Weapon|Firearm",
		meta = (EditCondition = "WeaponType == EWeaponType::WT_Firearm", EditConditionHides))
	FNAFirearmStatistics FirearmStatistics;
};
