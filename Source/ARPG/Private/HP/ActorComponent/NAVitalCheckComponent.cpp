// Fill out your copyright notice in the Description page of Project Settings.


#include "HP/ActorComponent/NAVitalCheckComponent.h"

#include "AbilitySystemComponent.h"
#include "NACharacter.h"
#include "Ability/AttributeSet/NAAttributeSet.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY( LogVitalComponent );

#define FIND_MATERIAL(T, VariableName, Path) \
	static ConstructorHelpers::FObjectFinder<T> Mat_##VariableName(TEXT(Path)); \
	if (Mat_##VariableName.Succeeded()) \
	{ \
		VariableName = Mat_##VariableName.Object; \
	}


// Sets default values for this component's properties
UNAVitalCheckComponent::UNAVitalCheckComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// 기본 머테리얼 저장
	FIND_MATERIAL( UMaterialInstance, GreenMaterial, "/Script/Engine.MaterialInstanceConstant'/Game/00_ProjectNA/05_Resource/01_Material/Issac/MI_health_green.MI_health_green'" )
	FIND_MATERIAL( UMaterialInstance, YellowMaterial, "/Script/Engine.MaterialInstanceConstant'/Game/00_ProjectNA/05_Resource/01_Material/Issac/MI_health_yellow.MI_health_yellow'" )
	FIND_MATERIAL( UMaterialInstance, RedMaterial, "/Script/Engine.MaterialInstanceConstant'/Game/00_ProjectNA/05_Resource/01_Material/Issac/MI_health_red.MI_health_red'" )
	FIND_MATERIAL( UMaterial, BlankMaterial, "/Script/Engine.Material'/Game/00_ProjectNA/05_Resource/01_Material/Issac/BlankMaterial.BlankMaterial'" )
}

ECharacterStatus UNAVitalCheckComponent::GetCharacterStatus() const
{
	return CharacterState;
}

// Called when the game starts
void UNAVitalCheckComponent::BeginPlay()
{
	Super::BeginPlay();

	if ( GetNetMode() != NM_DedicatedServer )
	{
		const ANACharacter* Character = GetCharacter(); // 체력 매쉬 등을 바꿔야 하므로 캐릭터만 호환 가능
		check( Character );
		
		// ASC의 체력 변화를 감지, 클라이언트 모두 업데이트
		Character->GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate( UNAAttributeSet::GetHealthAttribute() ).AddUObject( this, &UNAVitalCheckComponent::OnHealthChanged );

		// 초기 업데이트
		if (const UNAAttributeSet* AttributeSet = Cast<UNAAttributeSet>( Character->GetAbilitySystemComponent()->GetAttributeSet( UNAAttributeSet::StaticClass() ) ) )
		{
			ChangeHealthMesh( AttributeSet->GetHealth(), AttributeSet->Health.GetBaseValue() );
			HandleAlive( Character, AttributeSet->GetHealth() );
			HandleKnockDown( Character, AttributeSet->GetHealth()  );
			HandleDead( Character, AttributeSet->GetHealth()  );
		}
	}
}

void UNAVitalCheckComponent::SetState( const ECharacterStatus NewStatus )
{
	UpdateGameplayTag( NewStatus );
}

ANACharacter* UNAVitalCheckComponent::GetCharacter() const
{
	return Cast<ANACharacter>( GetOwner() );
}

void UNAVitalCheckComponent::UpdateGameplayTag( const ECharacterStatus NewState )
{
	if ( GetNetMode() != NM_Client )
	{
		FGameplayTagContainer NewTags;
		FGameplayTagContainer OldTags;

		UAbilitySystemComponent* AbilitySystemComponent = GetCharacter()->GetAbilitySystemComponent();
		check( AbilitySystemComponent );

		switch ( CharacterState )
		{
		case ECharacterStatus::Alive:
			OldTags.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.Alive" ) );
			break;
		case ECharacterStatus::KnockDown:
			OldTags.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.Alive" ) );
			OldTags.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.KnockDown" ) );
			break;
		case ECharacterStatus::Dead:
			OldTags.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.Dead" ) );
			break;
		}

		AbilitySystemComponent->RemoveReplicatedLooseGameplayTags( OldTags );
	
		switch ( NewState )
		{
		case ECharacterStatus::Alive:
			NewTags.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.Alive" ) );
			break;
		case ECharacterStatus::KnockDown:
			NewTags.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.Alive" ) );
			NewTags.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.KnockDown" ) );
			break;
		case ECharacterStatus::Dead:
			NewTags.AddTag( FGameplayTag::RequestGameplayTag( "Player.Status.Dead" ) );
			break;
		}
	
		AbilitySystemComponent->AddReplicatedLooseGameplayTags( NewTags );
	}

	// 클라이언트와 서버 개별로 업데이트 (리플리케이션에 의존하지 않음)
	CharacterState = NewState;
}

void UNAVitalCheckComponent::HandleKnockDown( const ANACharacter* Character, const float NewHealth )
{
	UNAMontageCombatComponent* CombatComponent = Character->GetComponentByClass<UNAMontageCombatComponent>();
	check( CombatComponent );

	if ( UCharacterMovementComponent* MovementComponent = Cast<UCharacterMovementComponent>( Character->GetMovementComponent() ) )
	{
		// 캐릭터가 쓰러졌었다가 다시 일어난 경우
		if ( CharacterState == ECharacterStatus::KnockDown && NewHealth > 0.f )
		{
			MovementComponent->MaxWalkSpeed = 600.f;
			SetState( ECharacterStatus::Alive );
			CombatComponent->SetActive( true );
		}

		// 캐릭터가 녹다운이 된 경우
		if ( NewHealth <= 0.f )
		{
			MovementComponent->MaxWalkSpeed = 150.f;
			SetState( ECharacterStatus::KnockDown );
			CombatComponent->SetActive( false );
		}
	}
}

void UNAVitalCheckComponent::HandleDead( const ANACharacter* Character, const float NewHealth )
{
	// 캐릭터가 녹다운 상태에서 사망 상태로 변한 경우
	if ( CharacterState == ECharacterStatus::KnockDown && NewHealth < -100.f )
	{
		if ( UCharacterMovementComponent* MovementComponent = Cast<UCharacterMovementComponent>( Character->GetMovementComponent() ) )
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->DisableMovement();
			SetState( ECharacterStatus::Dead );
			// todo: 플레이어 관전자 모드로 전환
		}
	}
}

void UNAVitalCheckComponent::HandleAlive( const ANACharacter* Character, const float NewHealth )
{
	UNAMontageCombatComponent* CombatComponent = Character->GetComponentByClass<UNAMontageCombatComponent>();
	check( CombatComponent );
	
	if ( NewHealth > 0.f )
	{
		SetState( ECharacterStatus::Alive );
		CombatComponent->SetActive( true );	
	}
}

void UNAVitalCheckComponent::OnHealthChanged( const FOnAttributeChangeData& OnAttributeChangeData )
{
	if ( const ANACharacter* Character = Cast<ANACharacter>( GetCharacter() ) )
	{
		// 클라이언트 + 서버 동기화 (클라이언트가 요청해도 서버쪽에서 컴포넌트가 꺼져있기 때문에 공격 불가 판정)
		HandleAlive( Character, OnAttributeChangeData.NewValue );
		HandleKnockDown( Character, OnAttributeChangeData.NewValue );
		HandleDead( Character, OnAttributeChangeData.NewValue );
		
		if ( const UNAAttributeSet* AttributeSet = Cast<UNAAttributeSet>( Character->GetAbilitySystemComponent()->GetAttributeSet( UNAAttributeSet::StaticClass() ) ) )
		{
			ChangeHealthMesh
			(
				OnAttributeChangeData.NewValue,
				AttributeSet->Health.GetBaseValue()
			);
		}
	}
}

void UNAVitalCheckComponent::ChangeHealthMesh( const float NewValue, const float MaxValue )
{
	constexpr int32 MaxHealthMesh = 4;
	
	const float NewRatio = NewValue / MaxValue;
	const int32 FillCount = static_cast<int32>( NewRatio / MeshHealthStep );
	check( FillCount <= MaxHealthMesh );

	const bool bShouldGreen = FillCount >= 3;
	const bool bShouldYellow = FillCount >= 2;
	const bool bShouldRed = FillCount >= 1;

	if (bShouldGreen)
	{
		State = EVitalState::Green;
	}
	else if (bShouldYellow)
	{
		State = EVitalState::Yellow;
	}
	else if (bShouldRed)
	{
		State = EVitalState::Red;
	}
	else
	{
		// Red 상태는 그대로 유지
		return;
	}

	constexpr std::pair GreenIndex = { 12, 13 };
	constexpr std::pair YellowIndex = { 10, 11 };
	constexpr std::pair RedIndex = { 8, 9 };
	
	std::pair<int32, int32> Target;
	UMaterialInstance* TargetMaterial = nullptr;
	TArray<std::pair<int32, int32>> Others;
		
	switch ( State )
	{
	case EVitalState::Green:
		Target = GreenIndex;
		TargetMaterial = GreenMaterial;
		Others.Emplace( YellowIndex );
		Others.Emplace( RedIndex );
		break;
	case EVitalState::Yellow:
		Target = YellowIndex;
		TargetMaterial = YellowMaterial;
		Others.Emplace( RedIndex );
		Others.Emplace( GreenIndex );
		break;
	case EVitalState::Red:
		Target = RedIndex;
		TargetMaterial = RedMaterial;
		Others.Emplace( GreenIndex );
		Others.Emplace( YellowIndex );
		break;
	default:
		check( false );
	}

	if ( USkeletalMeshComponent* SkeletalMeshComponent = GetCharacter()->GetComponentByClass<USkeletalMeshComponent>() )
	{
		constexpr int32 BottomIndex = 6;
		
		SkeletalMeshComponent->SetMaterial( BottomIndex, TargetMaterial );
		SkeletalMeshComponent->SetMaterial( Target.first, TargetMaterial );
		SkeletalMeshComponent->SetMaterial( Target.second, TargetMaterial );

		for (const auto& [ Inner, Outer ] : Others)
		{
			SkeletalMeshComponent->SetMaterial( Inner, BlankMaterial );
			SkeletalMeshComponent->SetMaterial( Outer, BlankMaterial );
		}
	}
}


// Called every frame
void UNAVitalCheckComponent::TickComponent( float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

