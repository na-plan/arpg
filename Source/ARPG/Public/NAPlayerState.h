// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NAPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, const int32, Previous, const int32, New);

/**
 * 
 */
UCLASS()
class ARPG_API ANAPlayerState : public APlayerState
{
	GENERATED_BODY()

	// PlayerState의 체력 델레게이션과 연동하기 위함
	friend class ANAPlayerController;

public:
	// 체력 변화 델레게이션, 변화 직전과 현재 체력을 전파함
	FOnHealthChanged OnHealthChanged;

	// 현재 체력
	int32 GetHealth() const { return Health; }

	// 최대 체력
	int32 GetMaxHealth() const { return MaxHealth; }

	// 체력 설정 함수
	void SetHealth(int32 NewHealth);

	// 체력 증가 함수, 음수가 주어질 경우 체력이 감소
	void IncreaseHealth(int32 Increment);

	// 체력 감소 함수, 음수가 주어질 경우 체력이 증가
	void DecreaseHealth(int32 Decrement);

	// 생존 상태 반환
	bool IsAlive() const;

protected:

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Pawn의 OnTakeAnyDamage를 확인하고 체력 감소 수행
	UFUNCTION()
	virtual void OnCharacterTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
	
private:
	UPROPERTY(VisibleAnywhere, Replicated, Blueprintable, Category="Health", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	int32 Health;

	// 최대 체력은 클라이언트와 서버가 동기화한다고 가정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
	int32 MaxHealth;
	
};
