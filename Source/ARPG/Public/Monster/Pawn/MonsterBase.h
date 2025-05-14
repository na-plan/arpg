// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../AI/MonsterAIController.h"
#include "Aicontroller.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Pawn.h"
#include "MonsterBase.generated.h"
USTRUCT()
struct ARPG_API FMonsterBaseTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:	// Base Resource
	UPROPERTY(EditAnywhere, Category = "Monster")
	USkeletalMesh* SkeletalMesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "Monster")
	FTransform MeshTransform = FTransform::Identity;

	//Make Base Capsule
	UPROPERTY(EditAnywhere, Category = "Monster")
	TSubclassOf<UShapeComponent> CollisionClass = UCapsuleComponent::StaticClass();
	UPROPERTY(EditAnywhere, Category = "Monster")
	float CollisionCapsuleRadius = 50.f;
	UPROPERTY(EditAnywhere, Category = "Monster")
	float CollisionCapsuleHalfHeight = 100.f;

	//Base AiController 
	UPROPERTY(EditAnywhere, Category = "Pawn|AI")
	TSubclassOf<AAIController> AIControllerClass = AMonsterAIController::StaticClass();

public: // Animation
	//임시 animinstance
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TSubclassOf<UAnimInstance> AnimClass;
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TArray<UAnimMontage*> HitReactMontage;
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TArray<UAnimMontage*> DieMontage;
	// 상하체 분리 use Upper boddy
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TArray<UAnimMontage*> AttackMontage;

public: //Type & Other Datatable
	// 일반 특수 엘리트 중간보스 보스 등등		기본공격만 / 스킬사용가능 / 특수 패턴 / 기믹 등 추가하고 싶은거 추가 가능하게조절 가능
	uint8 MonsterType = 0;

	// 타입에 따라서 Max Speed 를 조절 할수도 있어서 바꿀수 있게 함
	UPROPERTY(EditAnywhere, Category = "Pawn|Movement")
	float MovementMaxSpeed = 400.f;


	// 나중에 스킬 같은거 사용 할때 datatable 만들고 일반 몹 말고 자식에 
	//UPROPERTY(EditAnywhere, Category = "Pawn|Skill", meta = (RowType = "/Script/ARPG.SkillTableRow"))
	//FDataTableRowHandle OwnSkillData;
	
	// 나중에 아이템 드랍 같은거 할때 만들기
	//UPROPERTY(EditAnywhere, Category = "Pawn|Drop", meta = (RowType = "/Script/ARPG.ItemTableRow"))
	//FDataTableRowHandle OwnDropData;

};

UCLASS()
class ARPG_API AMonsterBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMonsterBase();
	virtual void SetData(const FDataTableRowHandle& InDataTableRowHandle);
protected:
	//Duplacte In Editor
	virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;
	virtual void PostLoad() override;
	virtual void PostLoadSubobjects(FObjectInstancingGraph* OuterInstanceGraph) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform);
	//Take damage Parts
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	UPROPERTY()
	TObjectPtr<UShapeComponent> CollisionComponent;

	// load Monster Data Target
	UPROPERTY(EditAnywhere, meta = (RowType = "/Script/ARPG.MonsterBaseTableRow"))
	FDataTableRowHandle MonsterDataTableRowHandle;

	// Make Better to Useful 사용하기 편하게 하려고 사용할 예정입니다
	FMonsterBaseTableRow* MonsterData;

	UFloatingPawnMovement* MovementComponent;

protected:
	UPROPERTY(VisibleAnywhere)
	UAnimInstance* AnimInstance;
};
