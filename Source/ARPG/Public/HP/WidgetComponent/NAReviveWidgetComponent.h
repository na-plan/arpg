// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Components/WidgetComponent.h"
#include "NAReviveWidgetComponent.generated.h"

DECLARE_DYNAMIC_DELEGATE_RetVal(float, FProgressValue);
DECLARE_LOG_CATEGORY_EXTERN( LogReviveWidget, Log, All );

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARPG_API UNAReviveWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Material", meta=(AllowPrivateAccess="true"))
	UMaterial* BaseMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Material", meta=(AllowPrivateAccess="true"))
	UMaterialInstanceDynamic* RuntimeInstance = nullptr;

	double StartInServerTime;

public:
	// Sets default values for this component's properties
	UNAReviveWidgetComponent();
	
protected:
	void OnReviveApplied( UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle );

	void OnReviveRemoved( const FActiveGameplayEffect& ActiveGameplayEffect );
	
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType,
	                            FActorComponentTickFunction* ThisTickFunction ) override;
};
