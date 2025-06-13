// Fill out your copyright notice in the Description page of Project Settings.


#include "NAGameStateBase.h"

#include "NAPlayerState.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"

void ANAGameStateBase::HandleRevive( APlayerState* PlayerState, ECharacterStatus CharacterStatus )
{
	++AlivePlayer;
	--KnockDownPlayer;
}

void ANAGameStateBase::HandleKnockDown( APlayerState* PlayerState, ECharacterStatus CharacterStatus )
{
	--AlivePlayer;
	++KnockDownPlayer;
	CheckAndHandleFailed();
}

void ANAGameStateBase::HandleDead( APlayerState* PlayerState, ECharacterStatus CharacterStatus )
{
	--KnockDownPlayer;
	CheckAndHandleFailed();
}

void ANAGameStateBase::OnRep_Failed()
{
	CheckAndHandleFailed();
}

void ANAGameStateBase::ShowFailedWidget() const
{
	FailedWidgetComponent->SetVisibility( true );
	// todo: failed widget에서 재시작을 누르면 레벨 재시작 -> RestartRound
}

void ANAGameStateBase::RestartRound() const
{
	if ( HasAuthority() )
	{
		GetWorld()->GetFirstPlayerController()->RestartLevel();
	}
}

void ANAGameStateBase::CheckAndHandleFailed()
{
	if ( AlivePlayer != 0 )
	{
		return;
	}

	if ( HasAuthority() )
	{
		bFailed = true;
	}

	ShowFailedWidget();
}

bool ANAGameStateBase::HasAnyoneDead() const
{
	return AlivePlayer != PlayerArray.Num();
}

ANAGameStateBase::ANAGameStateBase()
{
	FailedWidgetComponent = CreateDefaultSubobject<UWidgetComponent>( TEXT("FailedWidgetComponent") );
	FailedWidgetComponent->SetWidgetSpace( EWidgetSpace::Screen );
}

void ANAGameStateBase::BeginPlay()
{
	Super::BeginPlay();
	FailedWidgetComponent->SetVisibility( false );
}

void ANAGameStateBase::AddPlayerState(APlayerState* PlayerState)
{
	const int32 Index = PlayerArray.Num() % 2;
	Super::AddPlayerState(PlayerState);

	if (ANAPlayerState* CastedPlayerState = Cast<ANAPlayerState>(PlayerState))
	{
		if ( HasAuthority() )
		{
			// 임시 플레이어 에셋 지정.. 서버에서 지정하면 리플리케이션으로 클라이언트에서 업데이트
			const FName CharacterAssetNames[]  = { "Male", "Female" };
			CastedPlayerState->SetPossessAssetName(CharacterAssetNames[Index]);

			// 서버에서 플레이어 상태 추적
			CastedPlayerState->OnRevived.AddUObject( this, &ANAGameStateBase::HandleRevive );
			CastedPlayerState->OnKnockDown.AddUObject( this, &ANAGameStateBase::HandleKnockDown );
			CastedPlayerState->OnDead.AddUObject( this, &ANAGameStateBase::HandleDead );

			++AlivePlayer;
		}
	}
}

void ANAGameStateBase::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
	DOREPLIFETIME( ANAGameStateBase, bFailed );
}
