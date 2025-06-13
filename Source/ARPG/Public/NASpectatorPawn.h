// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "NASpectatorPawn.generated.h"

class ANACharacter;

UCLASS()
class ARPG_API ANASpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Target, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<const ANACharacter> TargetPawn;
	
public:
	// Sets default values for this pawn's properties
	ANASpectatorPawn();

protected:
	void AttachToOtherPlayerImpl( const APawn* Other );
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void UnPossessed() override;

	UFUNCTION(Client, Reliable)
	void Client_AttachToOtherPlayer( const APawn* OtherCharacter );

public:
	// Called every frame
	virtual void Tick( float DeltaTime ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent( class UInputComponent* PlayerInputComponent ) override;
};
