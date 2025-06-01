// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Spawner/MonsterSpawner.h"

#include "Assets/AssetStatics.h"
#include "Components/BoxComponent.h"


// Sets default values
AMonsterSpawner::AMonsterSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	//Trigger 만드는 과정
	UBoxComponent* CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetupAttachment(RootComponent);
	//Overlap All
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);

	/* 
	Monster Data 가지고 와서 Spawn할 대상 Spawn 하고 대상 죽으면 일정 시간후 다시 spawn하는 형식? 

	1. 일단 몬스터 spawn 시키는놈 하나 
	2. 이후 spawn을 시키는 놈들을 들고있는 큰 녀석이 하나 
	3. 2가 특정 트리거(Player가 일정 범위 안에 있을 경우) 몬스터를 spawn 이런 형식으로 스폰을 시키고
	4. 2에서 Player가 나가면 2가 보유한 범위안에 있는 몬스터들을 디스폰 ㄱㄱ
	5. 1에서 spawn 시킨 몬스터가 죽으면 respawn을 작동 -> respawntime이 지나면 respawn

	monster 여기에 넣기전 null -> monster 이름 넣고 
	일단 몬스터 부터 만들고 spawn시키자
	*/	

	
	

}

// Called when the game starts or when spawned
void AMonsterSpawner::BeginPlay()
{
	Super::BeginPlay();
	if (SpawnOneTime)
	{
		SpawnMonster(IsSpawn);
	}
}

#if WITH_EDITOR
void AMonsterSpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	//PreView
	if (PreviewSpawnTarget)
	{
		if (!PreviewActor)
		{
			PreviewActor = GetWorld()->SpawnActor<AActor>(PreviewSpawnTarget, GetActorLocation(), GetActorRotation());
			PreviewActor->SetActorEnableCollision(false);
			PreviewActor->SetActorHiddenInGame(true);
		}
		else
		{
			PreviewActor->SetActorLocation(GetActorLocation());
			PreviewActor->SetActorRotation(GetActorRotation());
		}
	}
}

void AMonsterSpawner::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	if (PreviewActor)
	{
		PreviewActor->SetActorLocation(GetActorLocation());
		PreviewActor->SetActorRotation(GetActorRotation());
	}
}

void AMonsterSpawner::PostDuplicate(EDuplicateMode::Type DuplicateMode)
{
	Super::PostDuplicate(DuplicateMode);
}

void AMonsterSpawner::PostLoad()
{
	Super::PostLoad();
}

void AMonsterSpawner::PostLoadSubobjects(FObjectInstancingGraph* OuterInstanceGraph)
{
	Super::PostLoadSubobjects(OuterInstanceGraph);
}
#endif

// Called every frame
void AMonsterSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsSpawn)
	{
		if (!SpawnOneTime)
		{
			if (LastSpawnTime >= SpawnTime)
			{
				SpawnMonster(IsSpawn);
				LastSpawnTime = 0.f; 
			}
			LastSpawnTime += DeltaTime;
		}
		else 
		{
			if (!IsSpawned)
			{
				SpawnMonster(IsSpawn);
				IsSpawned = true;
			}
		}

	}
}

void AMonsterSpawner::SpawnMonster(bool Spawncheck)
{
	//Spawn이 false면 spawn하지 못하도록
	if (!Spawncheck) { return; }

	const FVector& SpawnLocation = GetActorLocation();
	//Rotation도 돌아가도록 함
	const FRotator& SpawnRotation = GetActorRotation();

	//SpawnController 쪽에서 사용 예정
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	SpawnParameters.ObjectFlags = RF_Transient;

	// AssetName이 존재하지 않으면 터지는건 GetAssetClass 함수 사용해서 그런거라 지우는게 낫지 않을까? 싶기도 하고...
	//if (const TSubclassOf<AActor> Class = FAssetStatics::GetAssetClass(GetWorld(), AssetName))
	//{
	//	AActor* Spawned = GetWorld()->SpawnActor(Class, &SpawnLocation, &SpawnRotation, SpawnParameters);
	//	Spawned->SetReplicates(true);
	//}
	//else
	//{
	//	ensureAlwaysMsgf(Class, TEXT("Spawning class does not defined"));
	//}
	// AssetName을 사용하지 않을 경우 해당 주석을 풀어주세요
	if (const TSubclassOf<AActor> PreviewClass = PreviewSpawnTarget)
	{
		AActor* Spawned = GetWorld()->SpawnActor(PreviewClass, &SpawnLocation, &SpawnRotation);
		Spawned->SetReplicates(true);
	}
}

