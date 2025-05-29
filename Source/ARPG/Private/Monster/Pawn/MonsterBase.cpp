// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster/Pawn/MonsterBase.h"
#include "Net/UnrealNetwork.h"

//Timer
#include "AbilitySystemComponent.h"
#include "Ability/AttributeSet/NAAttributeSet.h"

//Ability
//#include "Ability/GameplayAbility/AttackGameplayAbility.h"
#include "Monster/Ability/GameplayAbility/GA_MonsterAttack.h"
#include "Monster/Ability/GameplayAbility/GA_Spawning.h"
#include "Monster/Ability/GameplayAbility/GA_UseSkill.h"

#include "Combat/ActorComponent/NAMontageCombatComponent.h"

#include "HP/GameplayEffect/NAGE_Damage.h"

//DEFINE_LOG_CATEGORY(LogTemplateMonster);

// Sets default values
AMonsterBase::AMonsterBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetupAttachment(RootComponent);
	CollisionComponent->SetCollisionObjectType( ECC_Pawn );

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/*TODO:: Not Use right now*/

	/*TODO:: After Create MovementComponents or Change MovementComponents 
	if someone Use UFloatingPawnMovement Delete this Comment And Used it plz or someone do not need this delete all this codes*/
	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	check(MovementComponent);

	/*Use this Afeter Create StateComponent*/
	//StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
	//check(StatusComponent);
	
	/*Use this Afeter Create SkillComponent(Finishkill) */
	//SkillComponent = CreateDefaultSubobject<USkillComponent>(TEXT("MonsterSkillComponent"));
	//check(SkillComponent);

	
	/*AI*/
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	AISenseConfig_Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("AISenseConfig_Sight"));
	AISenseConfig_Sight->DetectionByAffiliation.bDetectNeutrals = true;
	AISenseConfig_Sight->SightRadius = 800.f;
	AISenseConfig_Sight->LoseSightRadius = 1000.f;
	AISenseConfig_Sight->PeripheralVisionAngleDegrees = 120.f;
	AIPerceptionComponent->ConfigureSense(*AISenseConfig_Sight);
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	DefaultCombatComponent = CreateDefaultSubobject<UNAMontageCombatComponent>(TEXT("DefaultCombatComponent"));

	AutoPossessAI = EAutoPossessAI::Spawned;
}

void AMonsterBase::InitializeAbilities()
{
}

void AMonsterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		if (AbilitySystemComponent)
		{
			// 플레이어 스테이트 없이 몬스터가 처리
			AbilitySystemComponent->InitAbilityActorInfo(this, this);
		}

		SetOwner(NewController);
	}
}

// Called when the game starts or when spawned
void AMonsterBase::BeginPlay()
{
	Super::BeginPlay();

	//TSubclassOf<AAIController> MainAIControllerClass = AMonsterAIController::StaticClass();
	//AIControllerClass = MainAIControllerClass;

	if (AbilitySystemComponent) 
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UGA_MonsterAttack::StaticClass(), 1, 0));
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UGA_Spawning::StaticClass(), 1, 0));
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UGA_UseSkill::StaticClass(), 1, 0));

	}

}

void AMonsterBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMonsterBase, AbilitySystemComponent);
}
#pragma region Legacy
//Gas 전환 완료
//bool AMonsterBase::OnAttack()
//{
//	//AIControllerClass에서 OnAttack하도록 호출하고 AIControllerClass에서 가지고있는 component를 가져와 공격하도록 해야하나?
//	AIControllerClass;
//
//	return false;
//}
#pragma endregion

float AMonsterBase::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	//there is no Status Part so Skip Status Parts
	//TODO:: After Create Status Data or Components  Plz Add Here
	
	//if (StatusComponent->IsDie()) { return 0.f; }


	//float DamageResult = StatusComponent->TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	//if (FMath::IsNearlyZero(DamageResult)) { return 0.0; }

	if (Controller)
	{
		//damage 맞을때 멈칫하려고 하지 않는다면 해당 부분은 delete해주세요
		Controller->StopMovement();
	}
	/*TODO:: Afeter Create StatusComponent */
	//if (StatusComponent->IsDie() && !MonsterData->DieMontage.IsEmpty())
	//{
	//	if (Controller)
	//	{
	//		Controller->Destroy();
	//	}
	//	SetActorEnableCollision(false);

	//	const int64 Index = FMath::RandRange(0, MonsterData->DieMontage.Num() - 1);
	//	CurrentDieMontage = MonsterData->DieMontage[Index];

	//	AnimInstance->Montage_Play(CurrentDieMontage);
	//	UKismetSystemLibrary::K2_SetTimer(this, TEXT("OnDie"),
	//		MonsterData->DieMontage[Index]->GetPlayLength() - 0.5f, false);
	//}
	//else if (!StatusComponent->IsDie() && !MonsterData->HitReactMontage.IsEmpty())
	//{
	//	const int64 HitReactIndex = FMath::RandRange(0, MonsterData->HitReactMontage.Num() - 1);
	//	AnimInstance->Montage_Play(MonsterData->HitReactMontage[HitReactIndex]);
	//}
	return 0.0f;
}

void AMonsterBase::OnDie()
{

}

// Called every frame
void AMonsterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


