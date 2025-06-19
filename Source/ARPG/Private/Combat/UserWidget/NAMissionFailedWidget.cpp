#include "Combat/UserWidget/NAMissionFailedWidget.h"

#include "NAGameStateBase.h"
#include "NAPlayerController.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/HorizontalBox.h"

void UNAMissionFailedWidget::OnConfirmButtonClicked()
{
	if ( ANAGameStateBase* GameStateBase = Cast<ANAGameStateBase>( GetPlayerContext().GetGameState() ) )
	{
		if ( GetWorld()->GetNetMode() == NM_Client )
		{
			if ( ANAPlayerController* PlayerController = Cast<ANAPlayerController>( GetPlayerContext().GetPlayerController() ) )
			{
				PlayerController->Server_VoteForRestart( GetPlayerContext().GetPlayerState(), true );
			}
		}
		else
		{
			GameStateBase->VoteForRestart( GetPlayerContext().GetPlayerState(), true );
		}
	}
}

void UNAMissionFailedWidget::Disconnect()
{
	if ( ANAGameStateBase* GameStateBase = Cast<ANAGameStateBase>( GetPlayerContext().GetGameState() ) )
	{
		GameStateBase->RemoveFailedWidget();
		
		if ( ANAPlayerController* PlayerController = Cast<ANAPlayerController>( GetPlayerContext().GetPlayerController() ) )
		{
			PlayerController->ClientTravel( TEXT("/Game/00_ProjectNA/02_Level/Level_NALobby"), TRAVEL_Absolute );
		}
	}
}

void UNAMissionFailedWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	AdjustCheckboxCount();
	ConfirmButton->OnClicked.AddUniqueDynamic( this, &UNAMissionFailedWidget::OnConfirmButtonClicked );
	DisconnectButton->OnClicked.AddUniqueDynamic( this, &UNAMissionFailedWidget::Disconnect );
}

void UNAMissionFailedWidget::AdjustCheckboxCount()
{
	if ( const ANAGameStateBase* GameSateBase = Cast<ANAGameStateBase>( GetPlayerContext().GetGameState() ) )
	{
		for ( int32 i = 0; i < GameSateBase->PlayerArray.Num(); ++i )
		{
			HaveConfirmed.Add( NewObject<UCheckBox>( this, NAME_None, RF_Transient) );
			UCheckBox* CheckBox = HaveConfirmed.Last();
			CheckBox->SetCheckedState( ECheckBoxState::Unchecked );
			CheckBox->SetVisibility( ESlateVisibility::HitTestInvisible );
			
			CheckBoxHorizontalBox->AddChildToHorizontalBox( CheckBox );
		}
	}
}

void UNAMissionFailedWidget::AdjustCheckboxCount( const TArray<bool>& BaseArray )
{
	CheckBoxHorizontalBox->ClearChildren();
	HaveConfirmed.Empty();
	
	for ( int32 i = 0; i < BaseArray.Num(); ++i )
	{
		HaveConfirmed.Add( NewObject<UCheckBox>( this, NAME_None, RF_Transient) );
		UCheckBox* CheckBox = HaveConfirmed.Last();
		CheckBox->SetCheckedState( BaseArray[ i ] ? ECheckBoxState::Checked : ECheckBoxState::Unchecked );
		CheckBox->SetVisibility( ESlateVisibility::HitTestInvisible );

		CheckBoxHorizontalBox->AddChildToHorizontalBox( CheckBox );
	}
}

void UNAMissionFailedWidget::UpdateVoteArray( const TArray<bool>& Array )
{
	if ( HaveConfirmed.Num() != Array.Num() )
	{
		AdjustCheckboxCount( Array );
	}
	else
	{
		for ( int32 i = 0; i < Array.Num(); ++i )
		{
			HaveConfirmed[ i ]->SetCheckedState( Array[i] ? ECheckBoxState::Checked : ECheckBoxState::Unchecked );
		}	
	}
}
