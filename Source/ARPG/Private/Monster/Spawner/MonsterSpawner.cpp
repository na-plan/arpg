// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Spawner/MonsterSpawner.h"

#include "Assets/AssetStatics.h"


// Sets default values
AMonsterSpawner::AMonsterSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	
	/* 
	Monster Data 가지고 와서 Spawn할 대상 Spawn 하고 대상 죽으면 일정 시간후 다시 spawn하는 형식? 

	1. 일단 몬스터 spawn 시키는놈 하나 
	2. 이후 spawn을 시키는 놈들을 들고있는 큰 녀석이 하나 
	3. 2가 특정 트리거(Player가 일정 범위 안에 있을 경우) 몬스터를 spawn 이런 형식으로 스폰을 시키고
	4. 2에서 Player가 나가면 2가 보유한 범위안에 있는 몬스터들을 디스폰 ㄱㄱ
	5. 1에서 spawn 시킨 몬스터가 죽으면 respawn을 작동 -> respawntime이 지나면 respawn


	일단 몬스터 부터 만들고 spawn시키자
	*/	


}

// Called when the game starts or when spawned
void AMonsterSpawner::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMonsterSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (LastSpawnTime >= 10.f)
	{
		SpawnMonster();
		LastSpawnTime = 0.f; 
	}

	LastSpawnTime += DeltaTime;
}

void AMonsterSpawner::SpawnMonster()
{
	const FVector& SpawnLocation = GetActorLocation();
	const FRotator& SpawnRotation = FRotator::ZeroRotator;
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	SpawnParameters.ObjectFlags = RF_Transient;

	if (const TSubclassOf<AActor> Class = FAssetStatics::GetAssetClass(GetWorld(), AssetName))
	{
		AActor* Spawned = GetWorld()->SpawnActor(Class, &SpawnLocation, &SpawnRotation, SpawnParameters);
		Spawned->SetReplicates(true);
	}
	else
	{
		ensureAlwaysMsgf(Class, TEXT("Spawning class does not defined"));
	}
}

