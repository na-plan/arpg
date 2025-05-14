#pragma once

#include "Item/Consumable/NAConsumableItem.h"

#include "NAProjectile.generated.h"

class ANAFirearm;
USTRUCT()
struct FNAProjectileStatistics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Projectile Statistics")
	TSubclassOf<ANAFirearm> FirearmClass;	// 발사체를 발사하는 총기 클래스

	UPROPERTY(EditAnywhere, Category = "Projectile Statistics")
	float ProjectileCollisionRadius = 0.0f;	// 발사체 충돌 반경

	UPROPERTY(EditAnywhere, Category = "Projectile Statistics")
	float ProjectileSpeed = 0.0f;	 // 발사체 속도

	UPROPERTY(EditAnywhere, Category = "Projectile Statistics")
	float ProjectileDamage = 0.0f;	// 발사체 피해량
};

class UProjectileMovementComponent;
USTRUCT()
struct ARPG_API FNAProjectileTableRow : public FNAItemBaseTableRow
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<UProjectileMovementComponent> ProjectileMovementClass;	// 발사체 이동 컴포넌트 클래스

	UPROPERTY(EditAnywhere, Category = "Projectile")
	FNAProjectileStatistics ProjectileStatistics;
};

UCLASS()
class ARPG_API ANAProjectile : public ANAConsumableItem
{
	GENERATED_BODY()
};