// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "Components/ActorComponent.h"
#include "NACombatComponent.generated.h"

class UGameplayEffect;
class UGameplayAbility;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDoStartAttack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FShouldAttack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDoStopAttack);
DECLARE_LOG_CATEGORY_EXTERN(LogCombatComponent, Log, All);

/*
 * 캐릭터의 전투 수행 관련 추상 부모 컴포넌트
 * 몽타주, 히트 스캔 등의 상황에 따라 자식을 만들어 사용
 * 실제로 전투의 기능을 수행하는 객체에서 사용 (무기)
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	bool bCanGrab = false;

	// 공격을 시전하는 객체가 부모 객체인가?
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	bool bConsiderChildActor = false;

	// 공격이 끝나면 자동으로 다시 공격해야 하는가?
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	bool bReplay = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	TSubclassOf<UGameplayAbility> AttackAbility;

	//Suplex 사용
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	TSubclassOf<UGameplayAbility> GrabAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	TSubclassOf<UGameplayEffect> AmmoType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	float BaseDamage = 10.f;

	UPROPERTY( Replicated )
	FRotator AttackOrientation;

	FGameplayAbilitySpecHandle AbilitySpecHandle;

public:
	// Sets default values for this component's properties
	UNACombatComponent();

	virtual void SetActive(bool bNewActive, bool bReset = false) override;

	// 공격을 시작할 경우 사용하는 델레게이트 
	UPROPERTY(BlueprintAssignable)
	FDoStartAttack DoStartAttack;

	// 공격을 멈출 경우 사용하는 델레게이트
	UPROPERTY(BlueprintAssignable)
	FDoStopAttack DoStopAttack;

	void SetAttackAbility(const TSubclassOf<UGameplayAbility>& InAbility);
	
	void SetGrabAbility(const TSubclassOf<UGameplayAbility>& InAbility);

	TSubclassOf<UGameplayEffect> GetAmmoType() const;

	// 공격을 수행
	UFUNCTION()
	virtual void StartAttack();

	UFUNCTION()
	virtual APawn* GetAttacker() const;

	// 공격을 중단
	UFUNCTION()
	void StopAttack();
	
	// 공격이 가능한 상태인지 확인
	virtual bool IsAbleToAttack();

	bool IsAttacking() const { return bAttacking; }

	void SetConsiderChildActor( const bool InConsiderChildActor );

	TSubclassOf<UGameplayAbility> GetAttackAbility() const;

	FRotator GetAttackOrientation() const;

	float GetBaseDamage() const { return BaseDamage; }
	
	UFUNCTION( Server, Reliable )
	void Server_RequestAttackAbility();
	
protected:
	void OnAbilityEnded( const FAbilityEndedData& AbilityEndedData );

	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 공격 상태로 변환하는 함수
	void SetAttack(bool NewAttack);

	void UpdateAttackAbilityToASC( bool bOnlyRemove );

	// 클라이언트가 bCanAttack의 변화를 발견했을 경우
	UFUNCTION()
	virtual void OnRep_CanAttack();

	UFUNCTION( Server, Reliable )
	void Server_SyncAttack(const bool bFlag);
	
	void UpdateAttackOrientation();

	UFUNCTION( Server, Reliable )
	void Server_CommitAttackOrientation( const FRotator& Rotator ); 
	
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
