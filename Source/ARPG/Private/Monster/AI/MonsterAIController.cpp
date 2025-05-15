// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster/AI/MonsterAIController.h"

void AMonsterAIController::BeginPlay()
{

}

void AMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	// TODO:: there is No Component right Now Plz Add Components After Create monster Components


}

void AMonsterAIController::Tick(float DeltaTime)
{
}

void AMonsterAIController::CheckSpawnRadius()
{
	FVector FSpawnLocation = Blackboard->GetValueAsVector(TEXT("SpwanPosition"));
	APawn* OwningPawn = GetPawn();
	FVector OwningPawnLocation = OwningPawn->GetActorLocation();
	//이동 반경
	float Radius = 2000;

	float Distance = FVector::Dist(FSpawnLocation, OwningPawnLocation);
	if (Distance > Radius) { Blackboard->SetValueAsBool(TEXT("OutRangedSpawn"), true); }
	else { Blackboard->SetValueAsBool(TEXT("OutRangedSpawn"), false); }
}
