// Copyright Epic Games, Inc. All Rights Reserved.

#include "NACharacter.h"
#include "AbilitySystemComponent.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "NAPlayerState.h"
#include "RenderGraphResources.h"
#include "Ability/AttributeSet/NAAttributeSet.h"
#include "Ability/GameInstanceSubsystem/NAAbilityGameInstanceSubsystem.h"
#include "HP/GameplayEffect/NAGE_Damage.h"
#include "Net/UnrealNetwork.h"
#include "UObject/PropertyIterator.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AARPGCharacter

ANACharacter::ANACharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

void ANACharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	
	// == 테스트 코드 ==
	{
		if (HasAuthority())
		{
			// 데미지
			FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
			EffectContext.AddInstigator(GetController(), this);

			// Gameplay Effect CDO, 레벨?, ASC에서 부여받은 Effect Context로 적용할 효과에 대한 설명을 생성
			const FGameplayEffectSpecHandle DamageEffectSpec = AbilitySystemComponent->MakeOutgoingSpec(UNAGE_Damage::StaticClass(), 1, EffectContext);

			// 설명에 따라 효과 부여 (본인에게)
			const auto& Handle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*DamageEffectSpec.Data.Get());
			// 다른 대상에게...
			//AbilitySystemComponent->ApplyGameplayEffectSpecToTarget()
			check(Handle.WasSuccessfullyApplied());
		}
	}
	// ===============
}

void ANACharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->InitAbilityActorInfo(GetPlayerState(), this);
		}

		SetOwner(NewController);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ANACharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANACharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANACharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ANACharacter::RetrieveAsset(const AActor* InCDO)
{
	if (const ANACharacter* DefaultAsset = Cast<ANACharacter>(InCDO))
	{
		const FTransform Transform = DefaultAsset->GetMesh()->GetRelativeTransform();
		
		GetMesh()->SetRelativeTransform(Transform);
		GetMesh()->SetSkeletalMeshAsset(DefaultAsset->GetMesh()->GetSkeletalMeshAsset());
		GetMesh()->SetAnimInstanceClass(DefaultAsset->GetMesh()->GetAnimClass());

		// 리플리케이션을 위한 Mesh Offset
		CacheInitialMeshOffset(Transform.GetLocation(), Transform.Rotator() );

		if (HasAuthority())
		{
			if (const UNAAbilityGameInstanceSubsystem* AbilityGameInstanceSubsystem = GetGameInstance()->GetSubsystem<UNAAbilityGameInstanceSubsystem>())
			{
				if (const UDataTable* DataTable = AbilityGameInstanceSubsystem->GetAttributeDataTable(GetAssetName()); DataTable && AbilitySystemComponent)
				{
					for (const FAttributeDefaults& Attribute : DefaultAsset->GetAbilitySystemComponent()->DefaultStartingData)
					{
						if (!AbilitySystemComponent->GetAttributeSet(Attribute.Attributes))
						{
							AbilitySystemComponent->InitStats(Attribute.Attributes, DataTable);	
						}
					}
				}
				else
				{
					ensureMsgf(DataTable, TEXT("Designated table is not found"));
				}
			}
			else
			{
				ensureMsgf(AbilityGameInstanceSubsystem, TEXT("Ability game instance subsystem is not initialized"));
			}
		
		}

		for (TFieldIterator<FObjectProperty> It(StaticClass()); It; ++It)
		{
			const FObjectProperty* Property = *It;

			if (!Property)
			{
				continue;
			}
			
			if (Property->PropertyClass->IsChildOf(UInputMappingContext::StaticClass()))
			{
				const auto* Other = Cast<UInputMappingContext>(Property->GetObjectPropertyValue_InContainer(DefaultAsset));
				Property->SetObjectPropertyValue_InContainer(this, const_cast<UInputMappingContext*>(Other));
			}

			if (Property->PropertyClass->IsChildOf(UInputAction::StaticClass()))
			{
				const auto* Other = Cast<UInputAction>(Property->GetObjectPropertyValue_InContainer(DefaultAsset));
				Property->SetObjectPropertyValue_InContainer(this, const_cast<UInputAction*>(Other));
			}			 
		}
	}
}

void ANACharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ANACharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

bool ANACharacter::ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser) const
{
	if (const ANAPlayerState* CastedPlayerState = GetPlayerState<ANAPlayerState>())
	{
		// 플레이어가 살아있을 경우에만 데미지를 입음
		return Super::ShouldTakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser) && CastedPlayerState->IsAlive(); 
	}
	
	return Super::ShouldTakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

void ANACharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANACharacter, AbilitySystemComponent);
}
