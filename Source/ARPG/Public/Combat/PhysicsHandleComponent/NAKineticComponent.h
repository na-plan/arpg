// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayEffectTypes.h"
#include "InputMappingContext.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "NAKineticComponent.generated.h"


struct FEnhancedInputActionEventBinding;
class UInputAction;
class UNAKineticAttributeSet;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARPG_API UNAKineticComponent : public UPhysicsHandleComponent
{
	GENERATED_BODY()

	friend class UNAGA_KineticGrab;

	constexpr static float MeshHealthStep = 0.25f;
	
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Kinetic", meta=( AllowPrivateAccess=true ) )
	bool bIsGrab = false;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Kinetic", meta=( AllowPrivateAccess=true ) )
	FVector_NetQuantizeNormal ControlForward;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Kinetic", meta=( AllowPrivateAccess=true ) )
	float GrabDistance;

	FGameplayAbilitySpecHandle GrabSpecHandle;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Material", meta=( AllowPrivateAccess=true ) )
	UMaterialInstance* GreenMaterial;
	
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Material", meta=( AllowPrivateAccess=true ) )
	UMaterialInstance* YellowMaterial;
	
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Material", meta=( AllowPrivateAccess=true ) )
	UMaterialInstance* RedMaterial;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Binding", meta=( AllowPrivateAccess=true ) )
	UInputAction* GrabAction;
	
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Binding", meta=( AllowPrivateAccess=true ) )
	UInputAction* ThrowAction;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Binding", meta=( AllowPrivateAccess=true ) )
	UInputAction* DistanceAdjustAction;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Binding", meta=( AllowPrivateAccess=true ) )
	UInputMappingContext* KineticMappingContext;

	TWeakObjectPtr<APlayerController> OwningController;

	FEnhancedInputActionEventBinding* GrabActionBinding;

	FEnhancedInputActionEventBinding* ReleaseActionBinding;
	
	FEnhancedInputActionEventBinding* ThrowActionBinding;
	
	FEnhancedInputActionEventBinding* DistanceAdjustActionBinding;

public:
	// Sets default values for this component's properties
	UNAKineticComponent();
	
	UFUNCTION()
	void Grab();

	UFUNCTION()
	void Release();

	float GetRange() const;

	float GetMinHoldRange() const;

	float GetMaxHoldRange() const;

	float GetForce() const;
	
	float GetGrabDistance() const;

	FVector_NetQuantizeNormal GetControlForward() const;

	void ToggleGrabAbility( const bool bFlag );
	
	bool HasGrabbed() const;

	void SetOwningController( APlayerController* PlayerController );

	void ForceUpdateControlForward();

protected:
	
	void OnAPChanged( const FOnAttributeChangeData& OnAttributeChangeData );

	void OnAPChanged( float Old, float New );

	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void BindKineticKeys();

	void UnbindKineticKeys();

	const UNAKineticAttributeSet* GetAttributeSet() const;

	UFUNCTION()
	void Throw();

	void ThrowImpl() const;

	UFUNCTION( Server, Unreliable )
	void Server_Throw();

	void AdjustDistanceImpl( float Magnitude );

	UFUNCTION( Server, Unreliable, WithValidation )
	void Server_AdjustDistance( float Magnitude );
	
	void AdjustDistance( const FInputActionValue& InputActionValue );

public:
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType,
	                            FActorComponentTickFunction* ThisTickFunction ) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
