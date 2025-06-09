// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/PickableItemActor/NAWeaponAmmoBox.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ANAWeaponAmmoBox::ANAWeaponAmmoBox() : ANAPickableItemActor( FObjectInitializer::Get() )
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	PickupMode = EPickupMode::PM_Inventory;
}

// Called when the game starts or when spawned
void ANAWeaponAmmoBox::BeginPlay()
{
	Super::BeginPlay();

	ensure( AmmoEffectType );
}

bool ANAWeaponAmmoBox::ExecuteInteract_Implementation( AActor* InteractorActor )
{
	bool bResult = Super::ExecuteInteract_Implementation( InteractorActor );

	if ( bResult )
	{
		if ( const TScriptInterface<IAbilitySystemInterface>& Interface = InteractorActor )
		{
			// 총알 갯수만큼 반복해서 이펙트를 적용을 시도하고
			for ( int32 i = 0; i < AmmoCount; ++i )
			{
				if ( !bResult )
				{
					break;
				}
				
				const FGameplayEffectContextHandle& EffectContext = Interface->GetAbilitySystemComponent()->MakeEffectContext();
				const FActiveGameplayEffectHandle& ActiveHandle = Interface->GetAbilitySystemComponent()->ApplyGameplayEffectToSelf( AmmoEffectType.GetDefaultObject(), 1.f, EffectContext );
				bResult &= ActiveHandle.WasSuccessfullyApplied();
			}

			// 만약 하나라도 실패하면 다시 원상복귀
			if ( !bResult )
			{
				const FInheritedTagContainer& Container = Cast<UTargetTagsGameplayEffectComponent>( AmmoEffectType.GetDefaultObject()->FindComponent( UTargetTagsGameplayEffectComponent::StaticClass() ) )->GetConfiguredTargetTagChanges();
				Interface->GetAbilitySystemComponent()->RemoveActiveEffectsWithAppliedTags( Container.Added );
			}
		}
		else
		{
			bResult = false;
		}
	}
	
	return bResult;
}

void ANAWeaponAmmoBox::EndInteract_Implementation( AActor* InteractorActor )
{
	Super::EndInteract_Implementation( InteractorActor );

	// todo: 드랍하는 총알에 갯수 적용하기

	// 적용돼있던 총알 효과 모두 제거
	if ( const TScriptInterface<IAbilitySystemInterface>& Interface = InteractorActor )
	{
		const FInheritedTagContainer& Container = Cast<UTargetTagsGameplayEffectComponent>( AmmoEffectType.GetDefaultObject()->FindComponent( UTargetTagsGameplayEffectComponent::StaticClass() ) )->GetConfiguredTargetTagChanges();
		Interface->GetAbilitySystemComponent()->RemoveActiveEffectsWithAppliedTags( Container.Added );
	}
}

void ANAWeaponAmmoBox::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
	DOREPLIFETIME( ANAWeaponAmmoBox, AmmoCount );
	DOREPLIFETIME( ANAWeaponAmmoBox, AmmoEffectType );
}

// Called every frame
void ANAWeaponAmmoBox::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

