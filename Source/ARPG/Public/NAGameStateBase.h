// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NAGameStateBase.generated.h"

enum class ECharacterStatus : uint8;
class UWidgetComponent;
DECLARE_DYNAMIC_DELEGATE( FOnMissionRestartVotePassed );

UENUM()
enum class EGameMode : uint8
{
	Single,
	CoopMode,
	Max
};


class ANAPlayerState;
/**
 * 
 */
UCLASS()
class ARPG_API ANAGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status", meta = (AllowPrivateAccess = "true"))
	int32 AlivePlayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status", meta = (AllowPrivateAccess = "true"))
	int32 KnockDownPlayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Failed, Category = "Status", meta = (AllowPrivateAccess = "true"))
	bool bFailed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Widget", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* FailedWidgetComponent;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_RestartVoteArray, Category = "Widget", meta = (AllowPrivateAccess = "true") )
	TArray<bool> RestartVoteArray;

	UPROPERTY()
	TSubclassOf<UUserWidget> FailedWidgetClass;

	UFUNCTION()
	void HandleRevive( APlayerState* PlayerState, ECharacterStatus CharacterStatus );

	UFUNCTION()
	void HandleKnockDown( APlayerState* PlayerState, ECharacterStatus CharacterStatus );

	UFUNCTION()
	void HandleDead( APlayerState* PlayerState, ECharacterStatus CharacterStatus );

	void UpdateMissionFailedWidget() const;
	
	UFUNCTION()
	void OnRep_RestartVoteArray() const;

	UFUNCTION()
	void OnRep_Failed();
	
	void ShowFailedWidget() const;

	void RestartRound() const;

	UFUNCTION()
	void CheckAndHandleFailed();
	
public:
	
	FOnMissionRestartVotePassed OnMissionRestartVotePassed;
	
	ANAGameStateBase();
	
	bool HasAnyoneDead() const;

	bool IsFailed() const;

	void RemoveFailedWidget();
	
	void VoteForRestart( APlayerState* PlayerState, bool bValue );

protected:
	virtual void BeginPlay() override;
	
	// PlayerState를 추가하기 전에 플레이어의 캐릭터를 부여
	virtual void AddPlayerState(APlayerState* PlayerState) override;

	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	EGameMode CurrentGameMode;
	
};
