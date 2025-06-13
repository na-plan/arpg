// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPG/Public/NAPlayerState.h"

#include "AbilitySystemComponent.h"
#include "NACharacter.h"
#include "NAGameStateBase.h"
#include "NASpectatorPawn.h"
#include "Ability/AttributeSet/NAAttributeSet.h"
#include "Assets/Interface/NAManagedAsset.h"
#include "GameFramework/SpectatorPawn.h"
#include "HP/ActorComponent/NAVitalCheckComponent.h"
#include "Net/UnrealNetwork.h"

float ANAPlayerState::GetHealth() const
{
	// 캐릭터의 AbilitySystemComponent로부터 체력을 조회
	if (const ANACharacter* Character = Cast<ANACharacter>( GetPawn() ))
	{
		const UNAAttributeSet* AttributeSet = Cast<UNAAttributeSet>(Character->GetAbilitySystemComponent()->GetAttributeSet(UNAAttributeSet::StaticClass()));
		return AttributeSet->GetHealth();
	}

	// 관전자 상태
	if ( Cast<ASpectatorPawn>( GetPawn() ) )
	{
		return 0;
	}

	check(false);
	return 0.f;
}

int32 ANAPlayerState::GetMaxHealth() const
{
	// 캐릭터의 AbilitySystemComponent로부터 체력을 조회
	if (const ANACharacter* Character = Cast<ANACharacter>( GetPawn() ))
	{
		const UNAAttributeSet* AttributeSet = Cast<UNAAttributeSet>(Character->GetAbilitySystemComponent()->GetAttributeSet(UNAAttributeSet::StaticClass()));
		return AttributeSet->GetMaxHealth();
	}
	
	// 관전자 상태
	if ( Cast<ASpectatorPawn>( GetPawn() ) )
	{
		return 0;
	}

	check(false);
	return 0.f;
}

bool ANAPlayerState::IsAlive() const
{
	// 체력 컴포넌트로부터 확인
	if ( const ANACharacter* Character = Cast<ANACharacter>( GetPawn() ) )
	{
		const ECharacterStatus CharacterStatus = Character->GetComponentByClass<UNAVitalCheckComponent>()->GetCharacterStatus();
		return CharacterStatus == ECharacterStatus::Alive || CharacterStatus == ECharacterStatus::KnockDown;
	}
	
	// 관전자 상태
	if ( Cast<ASpectatorPawn>( GetPawn() ) )
	{
		return false;
	}

	check( false );
	return false;
}

bool ANAPlayerState::IsKnockDown() const
{
	// 체력 컴포넌트로부터 확인
	if ( const ANACharacter* Character = Cast<ANACharacter>( GetPawn() ) )
	{
		const ECharacterStatus CharacterStatus = Character->GetComponentByClass<UNAVitalCheckComponent>()->GetCharacterStatus();
		return CharacterStatus == ECharacterStatus::KnockDown;
	}
	
	// 관전자 상태
	if ( Cast<ASpectatorPawn>( GetPawn() ) )
	{
		return false;
	}

	check( false );
	return false;
}

void ANAPlayerState::SetPossessAssetName(const FName& AssetName)
{
	// 수정은 서버에서 가능하도록
	if ( HasAuthority() )
	{
		PossessAssetName = AssetName;
		UpdatePossessAssetByName();
	}
}

void ANAPlayerState::OnRep_PossessAssetName()
{
	UpdatePossessAssetByName();
}

void ANAPlayerState::UpdatePossessAssetByName()
{
	if ( const TScriptInterface<INAManagedAsset> Interface = GetPawn() )
	{
		Interface->SetAssetName( PossessAssetName );
	}

	// 중간에 에셋이 바뀐 경우이니, 다시 ASC를 업데이트
	if ( const TScriptInterface<IAbilitySystemInterface>& Interface = GetPawn() )
	{
		Interface->GetAbilitySystemComponent()->InitAbilityActorInfo( this, GetPawn() );
	}
}

void ANAPlayerState::OnPlayerStatusChanged( ECharacterStatus Old, ECharacterStatus New )
{
	if ( Old == ECharacterStatus::KnockDown && New == ECharacterStatus::Alive )
	{
		OnRevived.Broadcast( this, New );
	}

	if ( Old == ECharacterStatus::Alive && New == ECharacterStatus::KnockDown )
	{
		OnKnockDown.Broadcast( this, New );
	}

	if (Old == ECharacterStatus::KnockDown && New == ECharacterStatus::Dead )
	{
		OnDead.Broadcast( this, New );
	}
}

void ANAPlayerState::HandleDead( APlayerState* PlayerState, ECharacterStatus CharacterStatus )
{
	if ( CharacterStatus == ECharacterStatus::Dead )
	{
		// 플레이어가 사망시 관전자로 전환
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.bNoFail = true;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		SpawnParameters.Owner = PlayerState->GetPlayerController();

		if ( const APawn* ThisPawn = PlayerState->GetPawn() )
		{
			APawn* Spectator = GetWorld()->SpawnActor<ANASpectatorPawn>( ThisPawn->GetActorLocation(), ThisPawn->GetActorRotation(), SpawnParameters );
			PlayerState->GetPlayerController()->Possess( Spectator );
		}
	}
}

void ANAPlayerState::BindVitalDelegate()
{
	if ( const APawn* Pawn = GetPawn() )
	{
		if ( UNAVitalCheckComponent* VitalComponent = Pawn->GetComponentByClass<UNAVitalCheckComponent>() )
		{
			VitalComponent->OnCharacterStateChanged.AddUObject( this, &ANAPlayerState::OnPlayerStatusChanged );
		}
	}
}

void ANAPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	// 지금 폰에 접근할 수 있다면 델레게이션을 붙이고...
	BindVitalDelegate();
	
	if ( HasAuthority() )
	{
		// 나중에 폰이 바뀔때를 대비해서 델레게이션을 붙이는 델레게이션을 붙임 (?)
		TScriptDelegate VitalDelegate;
		VitalDelegate.BindUFunction(this, "BindVitalDelegate");
		OnPawnSet.AddUnique(VitalDelegate);

		OnDead.AddUObject( this, &ANAPlayerState::HandleDead );
	}
}

void ANAPlayerState::PostNetInit()
{
	Super::PostNetInit();
	
	if (GetNetMode() == NM_Client)
	{
		// 캐릭터는 기본 클래스를 주고 블루프린트로부터 복사해서 동적으로 적용
		// 캐릭터의 에셋이 중간에 바뀌어야 할 경우를 대비
		TScriptDelegate AssetDelegate;
		AssetDelegate.BindUFunction(this, "UpdatePossessAssetByName");
		OnPawnSet.AddUnique(AssetDelegate);

		// 서버와 동일하게 델레게이션 설정을 맞춤
		TScriptDelegate VitalDelegate;
		VitalDelegate.BindUFunction(this, "BindVitalDelegate");
		OnPawnSet.AddUnique(VitalDelegate);
	}
}

void ANAPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME( ANAPlayerState, PossessAssetName )
}
