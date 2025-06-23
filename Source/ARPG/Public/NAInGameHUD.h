// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NAInGameHUD.generated.h"

class UNAInGameWidget;
/**
 * 
 */
UCLASS()
class ARPG_API ANAInGameHUD : public AHUD
{
	GENERATED_BODY()

	ANAInGameHUD();

	void OnEscapeDown();
	
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UI, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UNAInGameWidget> InGameWidgetType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= UI, meta = (AllowPrivateAccess = "true"))
	UNAInGameWidget* InGameWidget;
};
