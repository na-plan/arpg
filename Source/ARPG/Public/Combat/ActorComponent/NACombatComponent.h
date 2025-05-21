// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NACombatComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_API UNACombatComponent : public UActorComponent
{
	GENERATED_BODY()

	// 현재 공격중인가?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bAttacking = false;

	// 공격을 할 수 있는 상태인가?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	bool bCanAttack = false;

	// 공격 가능 상태에 기여하는 확인 함수들
	UPROPERTY()
	TArray<TFunction<bool(AActor*)>> Predications;

	// 공격 시간에 기여하는 함수들
	UPROPERTY()
	TArray<TFunction<float(AActor*)>> TimerPredications;

	// 공격시 수행될 함수
	UPROPERTY()
	TFunction<void(AActor*)> AttackPredication;

	FTimerHandle AttackWaitHandle;

public:
	// Sets default values for this component's properties
	UNACombatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 공격이 가능한 상태인지 확인 (bCanAttack 업데이트)
	void UpdateCanAttack();

	// 공격 시도
	void SetAttack(bool NewAttack);

	// 공격 상태 클라이언트 -> 서버 상태 업데이트
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetAttack(const bool NewAttack);

	// 공격 상태 서버 -> 클라이언트 강제 업데이트
	UFUNCTION(Server, Reliable)
	void Client_SyncAttack(const bool NewFire);

	UFUNCTION()
	virtual void OnAttack();

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
