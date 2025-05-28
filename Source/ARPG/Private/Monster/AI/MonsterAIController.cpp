// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster/AI/MonsterAIController.h"

#include "AbilitySystemComponent.h"
#include "Perception/AISenseConfig_Sight.h"

#include "Monster/Pawn/MonsterBase.h"
#include "Ability/GameplayAbility/AttackGameplayAbility.h"
#include "Skill/DataTable/SkillTableRow.h"


void AMonsterAIController::BeginPlay()
{
	Super::BeginPlay();



}

void AMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	// TODO:: there is No Component right Now Plz Add Components After Create monster Components


	if (AMonsterBase* OwnerMonster = Cast<AMonsterBase>(GetPawn()))
	{
		if (UAbilitySystemComponent* ASC = OwnerMonster->GetAbilitySystemComponent())
		{
			bool OwnAbilitySystemComponent = true;
		}
	}

	/*AI Setting*/
	// BeginPlay는 Pawn이 만들어지기 전에 호출함으로 해당 위치로 호출 순서를 바꿨습니다
	if (!IsValid(BrainComponent))
	{
		//BT로 바꾸기
		UBehaviorTree* BehaviorTree = LoadObject<UBehaviorTree>(nullptr, TEXT("/Script/AIModule.BehaviorTree'/Game/01_ExternalAssets/TempResource/Monster/AI/BT_BaseMonster.BT_BaseMonster'"));
		check(BehaviorTree);
		RunBehaviorTree(BehaviorTree);
		//Spawn 위치 기준 일정 범위 이상 못나가게 하려고 할때 사용 가능합니다
		APawn* OwningPawn = GetPawn();
		FVector FSpawnLocation = OwningPawn->GetActorLocation();
		Blackboard->SetValueAsVector(TEXT("SpwanPosition"), FSpawnLocation);
		Blackboard->SetValueAsBool(TEXT("Spawning"), true);

	}

}

void AMonsterAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	//Montage play중이면 이동 멈추는거
	IsPlayingMontage();
	//Player 찾기
	FindPlayerByPerception();

	//Spawn 장소로 부터 일정 거리 떨어졌는지 확인하기
	CheckSpawnRadius();

	//Player와의 거리 확인
	CheckPlayerDistance();
}

void AMonsterAIController::CheckSpawnRadius()
{
	FVector FSpawnLocation = Blackboard->GetValueAsVector(TEXT("SpwanPosition"));
	APawn* OwningPawn = GetPawn();
	FVector OwningPawnLocation = OwningPawn->GetActorLocation();
	//이동 반경
	float Radius = 2000;

	float Distance = FVector::Dist(FSpawnLocation, OwningPawnLocation);
	if (Distance > Radius) 
	{ 
		Blackboard->SetValueAsBool(TEXT("OutRangedSpawn"), true); 
		Blackboard->SetValueAsObject(TEXT("DetectTarget"), nullptr);
	}
	else { Blackboard->SetValueAsBool(TEXT("OutRangedSpawn"), false); }
}

void AMonsterAIController::CheckPlayerDistance()
{
	UObject* DetectedPlayer = Blackboard->GetValueAsObject(TEXT("DetectPlayer"));
	if (DetectedPlayer != nullptr)
	{
		if (AActor* DetectedPlayerActor = Cast<AActor>(DetectedPlayer))
		{
			APawn* OwningPawn = GetPawn();

			FVector OwningPawnLocation = OwningPawn->GetActorLocation();
			FVector DetectedPlayerLocation = DetectedPlayerActor->GetActorLocation();
	
			float Distance = FVector::Dist(DetectedPlayerLocation, OwningPawnLocation);

			Blackboard->SetValueAsFloat(TEXT("PlayerDistance"), Distance);
		}
	}

}

void AMonsterAIController::FindPlayerByPerception()
{
	APawn* OwningPawn = GetPawn();
	if (UAIPerceptionComponent* AIPerceptionComponent = OwningPawn->GetComponentByClass<UAIPerceptionComponent>())
	{
		TArray<AActor*> OutActors;
		AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISenseConfig_Sight::StaticClass(), OutActors);
		bool bFound = false;
		
		// 시야에 보인 Player를 찾는 부분입니다. -> 이후 선공을 하며 
		// 중간에 다른 사람이 공격할 경우 그 사람을 공격하려고 합니다
		for (AActor* It : OutActors)
		{
			if (ACharacter* DetectedPlayer = Cast<ACharacter>(It))
			{
				bFound = true;
				Blackboard->SetValueAsObject(TEXT("DetectPlayer"), Cast<UObject>(DetectedPlayer));
				break;
			}
		}
		if (!bFound)
		{
			Blackboard->ClearValue(TEXT("DetectPlayer"));
		}
	}

}

void AMonsterAIController::IsPlayingMontage()
{
	APawn* OwningPawn = GetPawn();

	// Montage가 Play 중이라면 BT 내부에서 AI 진행을 멈춘다
	const bool bMontagePlaying = OwningPawn->GetComponentByClass<USkeletalMeshComponent>()->GetAnimInstance()->IsAnyMontagePlaying();


	const UAnimMontage* CurrentMontage = OwningPawn->GetComponentByClass<USkeletalMeshComponent>()->GetAnimInstance()->GetCurrentActiveMontage();
	const float CurrentPosition = OwningPawn->GetComponentByClass<USkeletalMeshComponent>()->GetAnimInstance()->Montage_GetPosition(CurrentMontage);
	const float MontageLength = OwningPawn->GetComponentByClass<USkeletalMeshComponent>()->GetAnimInstance()->Montage_GetPlayRate(CurrentMontage);
	bool StopAI = false;
	//A가 사용중이면 Stop, A가 사용중인데 0.2보다 낮으면 Play가 되어야함
	if (MontageLength > 0.2f && bMontagePlaying) { StopAI = true; }
	else { StopAI = false; }

	Blackboard->SetValueAsBool(TEXT("MontagePlaying"), StopAI);


	if (!StopAI)
	{
		Blackboard->SetValueAsBool(TEXT("UsingSkill"), false);
		Blackboard->SetValueAsBool(TEXT("OnDamage"), false);
	}

}
//임시 확인용 입니다
class UGA_UseSkill;

void AMonsterAIController::SelectSkill()
{
	if (AMonsterBase* OwnerMonster = Cast<AMonsterBase>(GetPawn()))
	{
		UAbilitySystemComponent* MonsterASC = OwnerMonster->GetAbilitySystemComponent();

		FDataTableRowHandle OwnSkillData =OwnerMonster->GetSkillData();
		FOwnSkillTable* Data = OwnSkillData.GetRow<FOwnSkillTable>(TEXT("MonsterSkillData"));
		//보유한 스킬들
		//0번 1개 있을때 1이 됩니다.
		uint8 MonsterOwnskillNum = Data->OwnSkillArray.Num();

		// 임시 0번 스킬
		float SkillRange = Data->OwnSkillArray[0].Range;
		float Distance = Blackboard->GetValueAsFloat(TEXT("PlayerDistance"));
		// 선택한 skill의 data를 가지고 와서 distance가 playerdistance 보다 적으면 사용 하도록 설정하면 ㄱㅊ을거 같음
		if (Distance < SkillRange)
		{
			Blackboard->SetValueAsBool(TEXT("CanUseSkill"), true);
		}

		// gas로 쿨타임 관리를 하고 사용을 하고
		if (MonsterOwnskillNum > 0)
		{
			
		}

	}
}

void AMonsterAIController::OnAttack()
{
	if (AMonsterBase* OwnerMonster = Cast<AMonsterBase>(GetPawn()))
	{
		//Casting성공
		UAbilitySystemComponent* MonsterAbilitySystemComponent = OwnerMonster->GetAbilitySystemComponent();

	}



}
