// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemActor/NAWeapon.h"

#include "Abilities/GameplayAbility.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "Item/ItemDataStructs/NAWeaponDataStructs.h"
#include "Item/EngineSubsystem/NAItemEngineSubsystem.h"

// Sets default values
ANAWeapon::ANAWeapon() : ANAPickableItemActor(FObjectInitializer::Get())
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if ( IsValid( UNAItemEngineSubsystem::Get() ) && UNAItemEngineSubsystem::Get()->IsItemMetaDataInitialized() )
	{
		CombatComponent = CreateDefaultSubobject<UNAMontageCombatComponent>( TEXT("CombatComponent") );
		// 무기가 캐릭터의 Child Actor로 부착될 것이기에 CombatComponent의 설정이 Parent Actor로 가도록
		CombatComponent->SetConsiderChildActor( true );
	}

	PickupMode = EPickupMode::PM_Holdable | EPickupMode::PM_Inventory;
}

// Called when the game starts or when spawned
void ANAWeapon::BeginPlay()
{
	Super::BeginPlay();

	// 몽타주랑 공격이 설정되어 있는지 확인
	check( CombatComponent->GetMontage() && CombatComponent->GetAttackAbility() );
	
}

// Called every frame
void ANAWeapon::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

