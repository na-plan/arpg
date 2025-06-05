// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemActor/NAPlaceableItemActor.h"
#include "NAPlaceableItemActor_Door.generated.h"

UENUM(Blueprintable)
enum class EDoorType : uint8
{
	UpDown,
	LeftRight,
	Max
};

DECLARE_DELEGATE_OneParam(FOnTickInteraction,AActor*);

UCLASS()
class ARPG_API ANAPlaceableItemActor_Door : public ANAPlaceableItemActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANAPlaceableItemActor_Door(const FObjectInitializer& ObjectInitializer);

	virtual void PostRegisterAllComponents() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents() override;

	//======================================================================================================================
	// Interactable Interface Implements
	//======================================================================================================================
public:
	virtual void BeginInteract_Implementation(AActor* Interactor) override;
	virtual void EndInteract_Implementation(AActor* Interactor) override;
	virtual void ExecuteInteract_Implementation(AActor* Interactor) override;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void InitInteraction(AActor* Interactor);

	void ActiveDoor();
	
protected:
	UPROPERTY()
	AActor* CurrentInteractingActor = nullptr;
	
	// 상하, 좌우 순으로 등록할 것
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door")
	UStaticMeshComponent* Door1 = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door")
	UStaticMeshComponent* Door2 = nullptr;

	UPROPERTY(EditAnywhere)
	EDoorType DoorType;

	UPROPERTY(EditAnywhere)
	double MoveDist = 300.0;
	
	UPROPERTY()
	float Duration = 1.5f;

	UPROPERTY()
	float CurrentTime = 0.f;

	UPROPERTY()
	FTransform OriginTF1;

	UPROPERTY()
	FTransform OriginTF2;

	// Delegate
	FOnTickInteraction TickInteraction;
};
