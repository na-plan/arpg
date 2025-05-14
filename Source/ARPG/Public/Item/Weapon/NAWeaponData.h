#pragma once

#include "Item/NAItemBaseData.h"

#include "NAWeaponData.generated.h"


UENUM(BlueprintType)
enum class EWeaponRarity : uint8
{
	WR_None        UMETA(DisplayName = "None"),        // �ʱ�ȭ �ʿ�

	WR_Common      UMETA(DisplayName = "Common"),      // �Ϲ� (���)
	WR_Uncommon    UMETA(DisplayName = "Uncommon"),    // ��� (���)
	WR_Rare        UMETA(DisplayName = "Rare"),        // ��� (�Ķ���)
	WR_Epic        UMETA(DisplayName = "Epic"),        // ���� (�����)
	WR_Legendary   UMETA(DisplayName = "Legendary"),   // ���� (��Ȳ��)
	WR_Mythic      UMETA(DisplayName = "Mythic"),      // ��ȭ (������ Ȥ�� �ݻ�)
};

UENUM(BlueprintType)
enum class  EWeaponType : uint8
{
	WT_None        UMETA(DisplayName = "None"),		// �ʱ�ȭ �ʿ�

	WT_Blunt       UMETA(DisplayName = "Blunt"),      // �б�: ������, ��ġ, ö�� ��
	WT_Blade       UMETA(DisplayName = "Blade"),      // ����: ��, ��, â �� ������
	WT_Firearm     UMETA(DisplayName = "Firearm")     // �ѱ�: ����, ����, ��ź�� ��
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
	TSubclassOf<ANAProjectile> ProjectileClass;	// �߻�ü Ŭ����

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	int32 MaxAmmo = -1;

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	float ReloadTime = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	float FireRate = 0.0f;	// �ʴ� �߻� ��, ���� �ӵ�

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	float BulletSpread = 0.0f;	// �Ѿ� ���� ����

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	float RecoilStrength = 0.0f;	// �ݵ� ����

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	float EffectiveRange = 0.0f;	// ��ȿ ��Ÿ�

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	int32 PierceCount = -1; 	// ���� ��, �Ѿ��� ���� �� ���� ������ �� �ִ���

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	int32 AmmoPerShot = -1; 	// �� �� �߻��� �� �Һ�Ǵ� ź ��

	UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	bool bIsAutomatic = false;	// �ڵ� �߻� ����

	//UPROPERTY(EditAnywhere, Category = "Firearm Statistics")
	//EFireMode FireMode = EFireMode::FM_Single;	// �߻� ��� (�ܹ�, ����, ���� ��)
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
