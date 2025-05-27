// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../AI/MonsterAIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Aicontroller.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "CoreMinimal.h"

#include "AbilitySystemInterface.h"
#include "Logging/LogMacros.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Pawn.h"
#include "MonsterBase.generated.h"


//DECLARE_LOG_CATEGORY_EXTERN(LogTemplateMonster, Log, All);

//Monster 도 경국 ability system을 사용을 해서 공격이나 다른걸 사용하니 얘도 component 붙여야 할거 같음
class UAbilitySystemComponent;


UCLASS()
class ARPG_API AMonsterBase : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMonsterBase();

	virtual void PossessedBy(AController* NewController) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/* Gas 전환중 */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	virtual void OnDie();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	//Gas에서 호출하는 함수들은 여기에 사용하는게 좋아보임
public:
	FORCEINLINE UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = Gameplay, meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere)
	UCapsuleComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UFloatingPawnMovement* MovementComponent;

protected:
	UPROPERTY(VisibleAnywhere)
	UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY(VisibleAnywhere)
	UAISenseConfig_Sight* AISenseConfig_Sight;

	float CheckTimer = 0;
	float CheckHP = 0;

};
