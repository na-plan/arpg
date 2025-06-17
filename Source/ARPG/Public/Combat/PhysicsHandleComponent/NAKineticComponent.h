// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayEffectTypes.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "NAKineticComponent.generated.h"


class UNAKineticAttributeSet;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARPG_API UNAKineticComponent : public UPhysicsHandleComponent
{
	GENERATED_BODY()

	friend class UNAGA_KineticGrab;

	constexpr static float MeshHealthStep = 0.25f;
	
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Kinetic", meta=( AllowPrivateAccess=true ) )
	bool bIsGrab = false;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Kinetic", meta=( AllowPrivateAccess=true ) )
	float GrabDistance;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Kinetic", meta=( AllowPrivateAccess=true ) )
	FVector_NetQuantizeNormal ActorForward;

	FGameplayAbilitySpecHandle GrabSpecHandle;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Material", meta=( AllowPrivateAccess=true ) )
	UMaterialInstance* GreenMaterial;
	
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Material", meta=( AllowPrivateAccess=true ) )
	UMaterialInstance* YellowMaterial;
	
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Material", meta=( AllowPrivateAccess=true ) )
	UMaterialInstance* RedMaterial;

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

	FVector_NetQuantizeNormal GetActorForward() const;

	void ToggleGrabAbility( const bool bFlag );
	
	void ForceUpdateActorForward();

protected:
	
	void OnAPChanged( const FOnAttributeChangeData& OnAttributeChangeData );

	void OnAPChanged( float Old, float New );

	// Called when the game starts
	virtual void BeginPlay() override;

	const UNAKineticAttributeSet* GetAttributeSet() const;

public:
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType,
	                            FActorComponentTickFunction* ThisTickFunction ) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
