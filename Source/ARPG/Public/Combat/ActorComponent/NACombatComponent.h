// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Components/ActorComponent.h"
#include "NACombatComponent.generated.h"

class UGameplayAbility;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDoStartAttack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FShouldAttack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDoStopAttack);
DECLARE_LOG_CATEGORY_EXTERN(LogCombatComponent, Log, All);

/*
 * 캐릭터의 전투 수행 관련 추상 부모 컴포넌트
 * 몽타주, 히트 스캔 등의 상황에 따라 자식을 만들어 사용
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Abstract)
class ARPG_API UNACombatComponent : public UActorComponent
{
	GENERATED_BODY()

	// 현재 공격중인가?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	bool bAttacking = false;

	// 공격을 할 수 있는 상태인가?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CanAttack, meta=(AllowPrivateAccess="true"))
	bool bCanAttack = false;

	// 공격이 끝나면 자동으로 다시 공격해야 하는가?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	bool bReplay = true;
	
	// 다음 자동 공격을 대기하는 타이머 핸들
	FTimerHandle AttackTimerHandler;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	TSubclassOf<UGameplayAbility> AttackAbility;

	FGameplayAbilitySpecHandle AbilitySpecHandle;

public:
	// Sets default values for this component's properties
	UNACombatComponent();

	// 공격을 시작할 경우 사용하는 델레게이트 
	UPROPERTY(BlueprintAssignable)
	FDoStartAttack DoStartAttack;

	// 공격을 멈출 경우 사용하는 델레게이트
	UPROPERTY(BlueprintAssignable)
	FDoStopAttack DoStopAttack;

	void SetAttackAbility(const TSubclassOf<UGameplayAbility>& InAbility);

	// 공격을 수행
	UFUNCTION()
	virtual void StartAttack();

	// 공격을 중단
	UFUNCTION()
	void StopAttack();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 공격이 가능한 상태인지 확인
	virtual bool IsAbleToAttack();

	// 공격 상태로 변환하는 함수
	void SetAttack(bool NewAttack);

	// 공격 값이 바뀌고 난 후 수행할 작업
	virtual void PostSetAttack();

	// 공격 상태 클라이언트 -> 서버 상태 업데이트 요청
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetAttack(const bool NewAttack);

	// 공격 상태 서버 -> 클라이언트 강제 업데이트
	UFUNCTION(Server, Reliable)
	void Client_SyncAttack(const bool NewFire);
	
	// 공격을 반복하는 함수
	UFUNCTION()
	void OnAttack();

	// 상속 후 공격 구현부
	UFUNCTION()
	virtual void OnAttack_Implementation() PURE_VIRTUAL(UNACombatComponent::OnAttack_Implemetation, );

	// 상속 후 공격 종료 시간 구현부
	UFUNCTION()
	virtual float GetNextAttackTime() PURE_VIRTUAL(UNACombatComponent::GetNextAttackTime, return 0.f;);

	// 클라이언트가 bCanAttack의 변화를 발견했을 경우
	UFUNCTION()
	virtual void OnRep_CanAttack();

	TSubclassOf<UGameplayAbility> GetAttackAbility() const;
	
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
