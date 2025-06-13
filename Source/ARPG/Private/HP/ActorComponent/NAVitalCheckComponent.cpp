// Fill out your copyright notice in the Description page of Project Settings.


#include "HP/ActorComponent/NAVitalCheckComponent.h"

#include "AbilitySystemComponent.h"
#include "NACharacter.h"
#include "Ability/AttributeSet/NAAttributeSet.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HP/GameplayEffect/NAGE_Dead.h"
#include "HP/GameplayEffect/NAGE_KnockDown.h"

DEFINE_LOG_CATEGORY( LogVitalComponent );

// Sets default values for this component's properties
UNAVitalCheckComponent::UNAVitalCheckComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// 기본 머테리얼 저장
	FIND_OBJECT( GreenMaterial, "/Script/Engine.MaterialInstanceConstant'/Game/00_ProjectNA/05_Resource/01_Material/Issac/MI_health_green.MI_health_green'", 0 );
	FIND_OBJECT( YellowMaterial, "/Script/Engine.MaterialInstanceConstant'/Game/00_ProjectNA/05_Resource/01_Material/Issac/MI_health_yellow.MI_health_yellow'", 1 );
	FIND_OBJECT( RedMaterial, "/Script/Engine.MaterialInstanceConstant'/Game/00_ProjectNA/05_Resource/01_Material/Issac/MI_health_red.MI_health_red'", 2 );
	FIND_OBJECT( BlankMaterial, "/Script/Engine.Material'/Game/00_ProjectNA/05_Resource/01_Material/Issac/BlankMaterial.BlankMaterial'", 3 );
}

ECharacterStatus UNAVitalCheckComponent::GetCharacterStatus() const
{
	return CharacterState;
}

void UNAVitalCheckComponent::OnMovementSpeedChanged( const FOnAttributeChangeData& OnAttributeChangeData )
{
	OnMovementSpeedChanged( OnAttributeChangeData.OldValue, OnAttributeChangeData.NewValue );
}

void UNAVitalCheckComponent::OnHealthChanged( const float Old, const float New )
{
	if ( const ANACharacter* Character = Cast<ANACharacter>( GetCharacter() ) )
	{
		// 클라이언트 + 서버 동기화 (클라이언트가 요청해도 서버쪽에서 컴포넌트가 꺼져있기 때문에 공격 불가 판정)
		HandleDead( Character, New );
		HandleKnockDown( Character, New );
		HandleAlive( Character, New );
		
		if ( const UNAAttributeSet* AttributeSet = Cast<UNAAttributeSet>( Character->GetAbilitySystemComponent()->GetAttributeSet( UNAAttributeSet::StaticClass() ) ) )
		{
			ChangeHealthMesh
			(
				New,
				AttributeSet->GetMaxHealth()
			);
		}
	}
}

void UNAVitalCheckComponent::OnMovementSpeedChanged( const float Old, const float New )
{
	if ( ANACharacter* Character = GetCharacter() )
	{
		Cast<UCharacterMovementComponent>( Character->GetMovementComponent() )->MaxWalkSpeed = New;
	}
}

void UNAVitalCheckComponent::HandleEffectToStatus( UAbilitySystemComponent* AbilitySystemComponent,
                                                   const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle )
{
	if ( GameplayEffectSpec.Def->IsA<UNAGE_Dead>() )
	{
		if ( UCharacterMovementComponent* MovementComponent = Cast<UCharacterMovementComponent>( GetCharacter()->GetMovementComponent() ) )
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->DisableMovement();
			SetState( ECharacterStatus::Dead );	
		}
	}
	else if ( GameplayEffectSpec.Def->IsA<UNAGE_KnockDown>() )
	{
		UNAMontageCombatComponent* CombatComponent = GetCharacter()->GetComponentByClass<UNAMontageCombatComponent>();
		check( CombatComponent );

		if ( UCharacterMovementComponent* MovementComponent = Cast<UCharacterMovementComponent>( GetCharacter()->GetMovementComponent() ) )
		{
			MovementComponent->SetMovementMode( MOVE_Walking );
		}
		
		SetState( ECharacterStatus::KnockDown );
		CombatComponent->SetActive( false );
	}
}

// Called when the game starts
void UNAVitalCheckComponent::BeginPlay()
{
	Super::BeginPlay();

	if ( GetNetMode() != NM_DedicatedServer )
	{
		const ANACharacter* Character = GetCharacter(); // 체력 매쉬 등을 바꿔야 하므로 캐릭터만 호환 가능
		check( Character );
		
		// ASC의 체력 변화를 감지
		if ( GetOwner()->HasAuthority() )
		{
			Character->GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate( UNAAttributeSet::GetHealthAttribute() ).AddUObject( this, &UNAVitalCheckComponent::OnHealthChanged );
			Character->GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate( UNAAttributeSet::GetMovementSpeedAttribute() ).AddUObject( this, &UNAVitalCheckComponent::OnMovementSpeedChanged );	
		}
		else
		{
			if ( const UNAAttributeSet* AttributeSet = Cast<UNAAttributeSet>( Character->GetAbilitySystemComponent()->GetAttributeSet( UNAAttributeSet::StaticClass() )  ))
			{
				AttributeSet->OnHealthChanged.AddUObject( this, &UNAVitalCheckComponent::OnHealthChanged );
				AttributeSet->OnMovementSpeedChanged.AddUObject( this, &UNAVitalCheckComponent::OnMovementSpeedChanged );
			}
		}

		Character->GetAbilitySystemComponent()->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject( this, &UNAVitalCheckComponent::HandleEffectToStatus );
		// 초기 업데이트
		if (const UNAAttributeSet* AttributeSet = Cast<UNAAttributeSet>( Character->GetAbilitySystemComponent()->GetAttributeSet( UNAAttributeSet::StaticClass() ) ) )
		{
			ChangeHealthMesh( AttributeSet->GetHealth(), AttributeSet->GetMaxHealth() );
			HandleAlive( Character, AttributeSet->GetHealth() );
			HandleKnockDown( Character, AttributeSet->GetHealth()  );
			HandleDead( Character, AttributeSet->GetHealth()  );
		}
	}
}

void UNAVitalCheckComponent::SetState( const ECharacterStatus NewStatus )
{
	const ECharacterStatus OldStatus = CharacterState;
	CharacterState = NewStatus;
	if ( OldStatus != NewStatus )
	{
		OnCharacterStateChanged.Broadcast( OldStatus, NewStatus );	
	}
}

ANACharacter* UNAVitalCheckComponent::GetCharacter() const
{
	return Cast<ANACharacter>( GetOwner() );
}

void UNAVitalCheckComponent::HandleKnockDown( const ANACharacter* Character, const float NewHealth )
{
	UNAMontageCombatComponent* CombatComponent = Character->GetComponentByClass<UNAMontageCombatComponent>();
	check( CombatComponent );

	// 캐릭터가 쓰러졌었다가 다시 일어난 경우
	if ( CharacterState == ECharacterStatus::KnockDown && NewHealth > 0.f )
	{
		SetState( ECharacterStatus::Alive );
		CombatComponent->SetActive( true );
		const FGameplayTagContainer TagContainer( FGameplayTag::RequestGameplayTag( "Player.Status.KnockDown" ) );
		Character->GetAbilitySystemComponent()->RemoveActiveEffectsWithAppliedTags( TagContainer );
	}

	// 캐릭터가 쓰러진 경우
	if ( CharacterState == ECharacterStatus::Alive && NewHealth <= 0.f )
	{
		CombatComponent->SetActive( false );

		if ( GetOwner()->HasAuthority() )
		{
			const FGameplayEffectContextHandle ContextHandle = Character->GetAbilitySystemComponent()->MakeEffectContext();
			const FGameplayEffectSpecHandle SpecHandle = Character->GetAbilitySystemComponent()->MakeOutgoingSpec( UNAGE_KnockDown::StaticClass(), 1.f, ContextHandle );
			const FActiveGameplayEffectHandle ActiveGameplayEffect = Character->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf( *SpecHandle.Data.Get() );
			check( ActiveGameplayEffect.WasSuccessfullyApplied() );	
		}
	}
}

void UNAVitalCheckComponent::HandleDead( const ANACharacter* Character, const float NewHealth )
{
	// 캐릭터가 녹다운 상태에서 사망 상태로 변한 경우
	if ( CharacterState == ECharacterStatus::KnockDown && NewHealth <= -100.f )
	{
		if ( GetOwner()->HasAuthority() )
		{
			const FGameplayEffectContextHandle ContextHandle = Character->GetAbilitySystemComponent()->MakeEffectContext();
			const FGameplayEffectSpecHandle SpecHandle = Character->GetAbilitySystemComponent()->MakeOutgoingSpec( UNAGE_Dead::StaticClass(), 1.f, ContextHandle );
			const FActiveGameplayEffectHandle ActiveGameplayEffect = Character->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf( *SpecHandle.Data.Get() );
			check( ActiveGameplayEffect.WasSuccessfullyApplied() );
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
	OnHealthChanged( OnAttributeChangeData.OldValue, OnAttributeChangeData.NewValue );
}

void UNAVitalCheckComponent::ChangeHealthMesh( const float NewValue, const float MaxValue )
{
	static constexpr int32 MaxHealthMesh = 4;
	
	const float NewRatio = NewValue / MaxValue;
	const int32 FillCount = NewRatio <= 0 ? 0 : static_cast<int32>( NewRatio / MeshHealthStep );
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

