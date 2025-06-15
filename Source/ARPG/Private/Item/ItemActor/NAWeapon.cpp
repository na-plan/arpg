// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemActor/NAWeapon.h"

#include "AbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/WidgetComponent/NAAmmoIndicatorComponent.h"

// Sets default values
ANAWeapon::ANAWeapon() : ANAPickableItemActor(FObjectInitializer::Get())
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CombatComponent = CreateDefaultSubobject<UNAMontageCombatComponent>( TEXT("CombatComponent") );
	// 무기가 캐릭터의 Child Actor로 부착될 것이기에 CombatComponent의 설정이 Parent Actor로 가도록
	CombatComponent->SetConsiderChildActor( true );

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>( TEXT("AbilitySystemComponent") );
	MuzzleFlashComponent = CreateDefaultSubobject<UNiagaraComponent>( TEXT( "MuzzleFlashComponent") );
	MuzzleFlashComponent->SetAutoActivate( false );

	AmmoIndicatorComponent = CreateDefaultSubobject<UNAAmmoIndicatorComponent>( TEXT("AmmoIndicatorComponent") );

	// 플레이어를 바라보도록
	AmmoIndicatorComponent->SetRelativeRotation( {0.f, -90.f, 0.f} );
	AmmoIndicatorComponent->SetRelativeLocation( {0.f, 0.f, 300.f} );
	
	PickupMode = /*EPickupMode::PM_Holdable | */EPickupMode::PM_Inventory;
}

// Called when the game starts or when spawned
void ANAWeapon::BeginPlay()
{
	Super::BeginPlay();

	// 몽타주랑 공격이 설정되어 있는지 확인
	check( CombatComponent->GetMontage() && CombatComponent->GetAttackAbility() );
	CombatComponent->SetActive( true );

	MuzzleFlashComponent->AttachToComponent( ItemMesh, FAttachmentTransformRules::KeepRelativeTransform, TEXT( "Muzzle" ) );
	MuzzleFlashComponent->SetActive( false );

	if ( !HasAuthority() )
	{
		if ( AActor* Actor = GetAttachParentActor() )
		{
			AbilitySystemComponent->InitAbilityActorInfo( Actor, this );

			// 클라이언트에 Child Actor 리플리케이션이 발생한 경우에 대한 대응
			// 만약 해당 무기 액터의 소유권자가 클라이언트 자신이라면 공격 Ability 부여 요청을 재시도
			if ( const APawn* Pawn = Cast<APawn>( Actor );
				 Pawn->GetController() == GetWorld()->GetFirstPlayerController() )
			{
				CombatComponent->Server_RequestAttackAbility();
			}
		}
	}
}

void ANAWeapon::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
	DOREPLIFETIME( ANAWeapon, CombatComponent );
	DOREPLIFETIME( ANAWeapon, AbilitySystemComponent );
}

void ANAWeapon::OnConstruction( const FTransform& Transform )
{
	Super::OnConstruction( Transform );

	AmmoIndicatorComponent->AttachToComponent( ItemMesh, FAttachmentTransformRules::KeepRelativeTransform, TEXT("Indicator") );
}

// Called every frame
void ANAWeapon::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

