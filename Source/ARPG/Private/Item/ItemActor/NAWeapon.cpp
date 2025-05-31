// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemActor/NAWeapon.h"

#include "Abilities/GameplayAbility.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "Item/ItemDataStructs/NAWeaponDataStructs.h"
#include "Item/Subsystem/NAItemEngineSubsystem.h"

void FCombatUpdatePredication::operator()( AActor* InOuter, UNAMontageCombatComponent* InComponent,
                                           UNAMontageCombatComponent* InOldComponent, const FNAItemBaseTableRow* InRow,
                                           const EItemMetaDirtyFlags DirtyFlags ) const
{
	if ( Cast<ANAWeapon>( InOuter ) )
	{
		const FNAWeaponTableRow* WeaponRow = static_cast<const FNAWeaponTableRow*>( InRow );
		InComponent->SetAttackMontage( WeaponRow->FirearmStatistics.FirearmMontage );
		InComponent->SetAttackAbility( WeaponRow->FirearmStatistics.GameplayAbility );
	}
}

#if WITH_EDITOR
void ANAWeapon::ExecuteItemPatch( UClass* ClassToPatch, const FNAItemBaseTableRow* PatchData,
                                  EItemMetaDirtyFlags PatchFlags )
{
	Super::ExecuteItemPatch(ClassToPatch, PatchData, PatchFlags);
	
	if (HasAnyFlags(RF_ClassDefaultObject) || !GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		return;
	}
	
	TArray<UActorComponent*> OldComponents;
	const EObjectFlags SubobjFlags = GetMaskedFlags(RF_PropagateToSubObjects) | RF_DefaultSubObject;
	
	FItemPatchHelper::UpdateDirtyComponent<
		UNAMontageCombatComponent,
		EItemMetaDirtyFlags::MF_Combat,
		FItemPatchHelper::FDefaultSpawnPredication<UNAMontageCombatComponent>,
		FCombatUpdatePredication>
	(
		PatchFlags,
		OldComponents,
		this,
		&CombatComponent,
		PatchData,
		TEXT( "CombatComponent" ),
		SubobjFlags,
		true
	);

	if (!OldComponents.IsEmpty())
	{
		for (UActorComponent* OldComponent : OldComponents)
		{
			OldComponent->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors);
			OldComponent->DestroyComponent();
			OldComponent = nullptr;
		}
		OldComponents.Empty();
	}
}
#endif

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

