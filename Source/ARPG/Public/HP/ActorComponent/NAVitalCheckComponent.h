// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "NACharacter.h"
#include "Components/ActorComponent.h"

#include "NAVitalCheckComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN( LogVitalComponent, Log, All );

// 체력 매시 상태
UENUM()
enum class EVitalState : uint8
{
	Green,
	Yellow,
	Red
};

UENUM()
enum class ECharacterStatus : uint8
{
	Alive,
	KnockDown,
	Dead
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARPG_API UNAVitalCheckComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Material", meta=(AllowPrivateAccess="true"))
	UMaterialInstance* GreenMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Material", meta=(AllowPrivateAccess="true"))
	UMaterialInstance* YellowMaterial;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Material", meta=(AllowPrivateAccess="true"))
	UMaterialInstance* RedMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Material", meta=(AllowPrivateAccess="true"))
	UMaterial* BlankMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status", meta=(AllowPrivateAccess="true"))
	EVitalState State = EVitalState::Green;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status", meta=(AllowPrivateAccess="true"))
	ECharacterStatus CharacterState = ECharacterStatus::Alive;


public:
	// Sets default values for this component's properties
	UNAVitalCheckComponent();

	ECharacterStatus GetCharacterStatus() const;

protected:
	// 매시에서 체력 구분 단위
	constexpr static float MeshHealthStep = 0.25f;
	
	// Called when the game starts
	virtual void BeginPlay() override;

	void SetState( ECharacterStatus NewStatus );

	ANACharacter* GetCharacter() const;

	void UpdateGameplayTag( const ECharacterStatus NewState );

	// 캐릭터가 쓰러졌을때 캐릭터 속성 관리
	void HandleKnockDown( const ANACharacter* Character, const float NewHealth );

	// 캐릭터가 죽었을때 캐릭터 속성 관리
	void HandleDead( const ANACharacter* Character, const float NewHealth );

	// 캐릭터가 살아있는 경우 캐릭터 속성 관리
	void HandleAlive( const ANACharacter* Character, const float NewHealth );
	
	// 캐릭터의 체력이 변화할 경우
	void OnHealthChanged( const FOnAttributeChangeData& OnAttributeChangeData );

	// 캐릭터의 체력 매시를 수정
	void ChangeHealthMesh( float NewValue, float MaxValue );
	
public:
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType,
	                            FActorComponentTickFunction* ThisTickFunction ) override;
};
