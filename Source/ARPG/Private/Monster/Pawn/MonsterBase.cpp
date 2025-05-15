// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster/Pawn/MonsterBase.h"

// Sets default values
AMonsterBase::AMonsterBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));

	SkeletalMeshComponent->SetupAttachment(DefaultSceneRoot);
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


}

void AMonsterBase::SetData(const FDataTableRowHandle& InDataTableRowHandle)
{
	MonsterDataTableRowHandle = InDataTableRowHandle;
	if (MonsterDataTableRowHandle.IsNull()) { return; }
	FMonsterBaseTableRow* Data = MonsterDataTableRowHandle.GetRow<FMonsterBaseTableRow>(TEXT("Enemy"));
	if (!Data) { ensure(false); return; }
	MonsterData = Data;
	
	//AI Controller Parts 
	/*Add AI After Create*/
	AIControllerClass = MonsterData->AIControllerClass;

	//Speed Control from Monster Data
	/*UFloatingPawnMovement 말고 다른거 사용할 경우 MovementComponent의 해당 타입을 사용할 다른걸로 바꿔주세요*/
	MovementComponent->MaxSpeed = MonsterData->MovementMaxSpeed;

	SkeletalMeshComponent->SetSkeletalMesh(MonsterData->SkeletalMesh);
	SkeletalMeshComponent->SetAnimClass(MonsterData->AnimClass);
	SkeletalMeshComponent->SetRelativeTransform(MonsterData->MeshTransform);

	/*Collision Component를 아직 안만들었음으로 넣지는 않음*/
	if (!IsValid(CollisionComponent) || CollisionComponent->GetClass() != MonsterData->CollisionClass)
	{
		EObjectFlags SubobjectFlags = GetMaskedFlags(RF_PropagateToSubObjects) | RF_DefaultSubObject;
		CollisionComponent = NewObject<UShapeComponent>(this, MonsterData->CollisionClass, TEXT("CollisionComponent"), SubobjectFlags);
		CollisionComponent->RegisterComponent();
		CollisionComponent->SetCanEverAffectNavigation(false);
		// Enigine Setting Collision 에 Monster가 없고 해당 설정이 없어서 일단 Pawn으로 합니다
		CollisionComponent->SetCollisionProfileName(TEXT("Pawn"));
		SetRootComponent(CollisionComponent);
		DefaultSceneRoot->SetRelativeTransform(FTransform::Identity);
		DefaultSceneRoot->AttachToComponent(CollisionComponent, FAttachmentTransformRules::KeepRelativeTransform);

		//Forcheck visable true
		CollisionComponent->SetVisibility(true);
	}

	if (UCapsuleComponent* CapsuleComponent = Cast<UCapsuleComponent>(CollisionComponent))
	{
		CapsuleComponent->SetCapsuleSize(MonsterData->CollisionCapsuleRadius, MonsterData->CollisionCapsuleHalfHeight);
	}
}

void AMonsterBase::SetSkillData(const FDataTableRowHandle& InSkillDataTableRowHandle)
{
	MonsterData->OwnSkillData = InSkillDataTableRowHandle;
	if (MonsterData->OwnSkillData.IsNull()) { return; }

	/*아직 미완성 부분입니다*/
	//FSkillTableRow* Data = MonsterData->OwnSkillData.GetRow<FSkillTableRow>(TEXT("MonsterSkillData"));
	//if (!Data) { ensure(false); return; }
	//MonsterSkillData = Data;

	//여기에 skill data setting중이라..
	//SkillComponent->SetData(InSkillDataTableRowHandle);


	//int32 CheckSkillNum = MonsterSkillData->SkillDataArray.Num();
}

void AMonsterBase::PostDuplicate(EDuplicateMode::Type DuplicateMode)
{
	Super::PostDuplicate(DuplicateMode);
	if (DuplicateMode == EDuplicateMode::Normal)
	{
		FTransform Backup = GetActorTransform();
		//CollisionComponent->DestroyComponent();
		SetData(MonsterDataTableRowHandle);
		SetActorTransform(Backup);
	}
}

void AMonsterBase::PostLoad()
{
	Super::PostLoad();
}

void AMonsterBase::PostLoadSubobjects(FObjectInstancingGraph* OuterInstanceGraph)
{
	Super::PostLoadSubobjects(OuterInstanceGraph);
}

// Called when the game starts or when spawned
void AMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	SetData(MonsterDataTableRowHandle);
}

void AMonsterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetData(MonsterDataTableRowHandle);
	SetActorTransform(Transform);
}

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


