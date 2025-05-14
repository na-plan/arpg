#pragma once

#include "Item/Consumable/NAConsumableItem.h"

#include "NAProjectile.generated.h"

class ANAFirearm;
USTRUCT()
struct FNAProjectileStatistics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Projectile Statistics")
	TSubclassOf<ANAFirearm> FirearmClass;	// �߻�ü�� �߻��ϴ� �ѱ� Ŭ����

	UPROPERTY(EditAnywhere, Category = "Projectile Statistics")
	float ProjectileCollisionRadius = 0.0f;	// �߻�ü �浹 �ݰ�

	UPROPERTY(EditAnywhere, Category = "Projectile Statistics")
	float ProjectileSpeed = 0.0f;	 // �߻�ü �ӵ�

	UPROPERTY(EditAnywhere, Category = "Projectile Statistics")
	float ProjectileDamage = 0.0f;	// �߻�ü ���ط�
};

class UProjectileMovementComponent;
USTRUCT()
struct ARPG_API FNAProjectileTableRow : public FNAItemBaseTableRow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<UProjectileMovementComponent> ProjectileMovementClass;	// �߻�ü �̵� ������Ʈ Ŭ����

	UPROPERTY(EditAnywhere, Category = "Projectile")
	FNAProjectileStatistics ProjectileStatistics;
};

UCLASS()
class ARPG_API ANAProjectile : public ANAConsumableItem
{
	GENERATED_BODY()
};