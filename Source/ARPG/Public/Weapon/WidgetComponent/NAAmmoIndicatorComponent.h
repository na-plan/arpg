// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayEffect.h"
#include "Components/WidgetComponent.h"
#include "NAAmmoIndicatorComponent.generated.h"


class UNAAmmoWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARPG_API UNAAmmoIndicatorComponent : public UWidgetComponent
{
	GENERATED_BODY()

	UPROPERTY()
	UMaterial* BackgroundMaterial;
	
	UPROPERTY()
	UMaterial* FrameMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* InstancedBackgroundMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* InstancedFrameMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	int32 CurrentAmmo = 0;
	
	FActiveGameplayEffectHandle TrackingAmmoHandle;

	static FLinearColor EnoughColor;
	static FLinearColor LowColor;

public:
	// Sets default values for this component's properties
	UNAAmmoIndicatorComponent();

protected:
	UFUNCTION()
	void UpdateAmmoCount( FActiveGameplayEffectHandle ActiveGameplayEffectHandle, int I, int Arg );

	UFUNCTION()
	FText GetAmmoCount();
	
	void UpdateAmmoCount( const int32 Count );

	void InitializeWidgetMaterial( UMaterialInstanceDynamic* Frame, UMaterialInstanceDynamic* Background ) const;

	UFUNCTION()
	void HandleAmmoAdded( UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle );

	UFUNCTION()
	void CleanupAmmoAdded( const FActiveGameplayEffect& ActiveGameplayEffect );
	
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType,
	                            FActorComponentTickFunction* ThisTickFunction ) override;
};
