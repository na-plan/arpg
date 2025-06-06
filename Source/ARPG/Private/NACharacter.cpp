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
#include "Algo/RemoveIf.h"
#include "ARPG/ARPG.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "Components/SplineComponent.h"
#include "EntitySystem/MovieSceneEntityManager.h"
#include "HP/ActorComponent/NAVitalCheckComponent.h"
#include "HP/GameplayAbility/NAGA_Revive.h"
#include "HP/GameplayEffect/NAGE_Damage.h"
#include "HP/WidgetComponent/NAReviveWidgetComponent.h"
#include "Net/UnrealNetwork.h"

#include "Interaction/NAInteractionComponent.h"
#include "Inventory/NAInventoryComponent.h"


DEFINE_LOG_CATEGORY( LogTemplateCharacter );

//////////////////////////////////////////////////////////////////////////
// AARPGCharacter

// 공격시 반복되는 함수 패턴
bool TryAttack( const AActor* Hand )
{
	if ( Hand )
	{
		if ( UNAMontageCombatComponent* CombatComponent = Hand->GetComponentByClass<UNAMontageCombatComponent>() )
		{
			if ( CombatComponent->IsAbleToAttack() )
			{
				CombatComponent->StartAttack();
				return true;
			}
		}
	}

	return false;
}

bool TryStopAttack( const AActor* Hand )
{
	if ( Hand )
	{
		if ( UNAMontageCombatComponent* CombatComponent = Hand->GetComponentByClass<UNAMontageCombatComponent>() )
		{
			if ( CombatComponent->IsAttacking() )
			{
				CombatComponent->StopAttack();
				return true;
			}
		}
	}

	return false;
}

ANACharacter::ANACharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionObjectType( ECC_Pawn );
		
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
	DefaultCombatComponent = CreateDefaultSubobject<UNAMontageCombatComponent>( TEXT( "DefaultCombatComponent" ) );
	InteractionComponent = CreateDefaultSubobject<UNAInteractionComponent>(TEXT("InteractionComponent"));
	InventoryWidgetBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("InventorySpringArm"));
	InventoryWidgetBoom->SetupAttachment(RootComponent);
	InventoryWidgetBoom-> bUsePawnControlRotation = false;
	InventoryWidgetBoom-> bInheritPitch = false;
	InventoryWidgetBoom-> bInheritYaw = false;
	InventoryWidgetBoom-> bInheritRoll = false;
	InventoryWidgetBoom->SetRelativeRotation(FRotator(0.0f, 190.0f, 0.0f));
	InventoryWidgetBoom->TargetArmLength = 10.f;
	InventoryWidgetBoom->bDoCollisionTest = false;
	InventoryComponent = CreateDefaultSubobject<UNAInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->SetupAttachment(InventoryWidgetBoom, USpringArmComponent::SocketName);
	InventoryCamOrbitSpline = CreateDefaultSubobject<USplineComponent>(TEXT("InventoryCamOrbitSpline"));
	
	LeftHandChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("LeftHandChildActor"));
	RightHandChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("RightHandChildActor"));

	VitalCheckComponent = CreateDefaultSubobject<UNAVitalCheckComponent>(TEXT("VitalCheckComponent"));
	ReviveWidget = CreateDefaultSubobject<UNAReviveWidgetComponent>( TEXT("ReviveWidgetComponent") );

	ApplyAttachments();
	
	GetMesh()->SetIsReplicated( true );
	bReplicates = true;
	ACharacter::SetReplicateMovement( true );

	AbilitySystemComponent->SetNetAddressable();
	DefaultCombatComponent->SetNetAddressable();
	LeftHandChildActor->SetNetAddressable();
	RightHandChildActor->SetNetAddressable();
}

void ANACharacter::ApplyAttachments() const
{
	TFunction<bool(USkeletalMeshComponent*, USceneComponent*, const FName&)> Attacher;
	if ( FUObjectThreadContext::Get().IsInConstructor )
	{
		Attacher = [](USkeletalMeshComponent* Parent, USceneComponent* Component, const FName& SocketName)
		{
			Component->SetupAttachment( Parent, SocketName );
			return true;
		};
	}
	else
	{
		Attacher = [](USkeletalMeshComponent* Parent, USceneComponent* Component, const FName& SocketName)
		{
			return Component->AttachToComponent( Parent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName );
		};
	}
	
	if ( GetMesh()->GetSkeletalMeshAsset() )
	{
		Attacher(GetMesh(), LeftHandChildActor, LeftHandSocketName);
		Attacher(GetMesh(), RightHandChildActor, RightHandSocketName);
		Attacher(GetMesh(), ReviveWidget, TEXT("ReviveWidgetSocket"));
	}
}

void ANACharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// 클라이언트의 BeginPlay에 맞춰 직접 RPC로 요청
	// 서버에서 직접 수행할 경우 클라이언트에서의 순서:
	// - Character -> BeginPlay -> GiveAbility -> PlayerState -> AbilityComponent 초기화
	// 초기화가 되지 않은 시점에서의 GiveAbility의 Replication을 받은 Client은 제대로된 값을 받지 못함
	if ( GetController() == GetWorld()->GetFirstPlayerController() )
	{
		Server_RequestReviveAbility();
	}
	
	// == 테스트 코드 ==
	{
		if (HasAuthority())
		{
			// 기본 공격이 정의되어있지 않음!
			check( DefaultCombatComponent->GetMontage() && DefaultCombatComponent->GetAttackAbility() );
			
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

void ANACharacter::PostNetInit()
{
	Super::PostNetInit();
}

void ANACharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if ( HasAuthority() )
	{
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->InitAbilityActorInfo(GetPlayerState(), this);
		}

		SetOwner(NewController);
	}

	if (InventoryComponent && NewController->IsLocalPlayerController())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(NewController))
		{
			if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
			{
				InventoryComponent->SetOwnerPlayer(LocalPlayer);
			}
		}
	}
}

void ANACharacter::Server_RequestReviveAbility_Implementation()
{
	// 부활 기능 추가
	const FGameplayAbilitySpec SpecHandle( UNAGA_Revive::StaticClass(), 1.f, static_cast<int32>( EAbilityInputID::Revive ) );
	AbilitySystemComponent->GiveAbility( SpecHandle );
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
			Subsystem->AddMappingContext(InventoryMappingContext, 1);
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

		EnhancedInputComponent->BindAction(LeftMouseAttackAction, ETriggerEvent::Started, this, &ANACharacter::StartLeftMouseAttack);
		EnhancedInputComponent->BindAction(LeftMouseAttackAction, ETriggerEvent::Completed, this, &ANACharacter::StopLeftMouseAttack);
		
		EnhancedInputComponent->BindAction( ReviveAction, ETriggerEvent::Started, this, &ANACharacter::TryRevive );
		EnhancedInputComponent->BindAction( ReviveAction, ETriggerEvent::Completed, this, &ANACharacter::StopRevive );
		// Interact
		EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Started, this, &ANACharacter::TryInteract);
		// Inventory
		EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &ANACharacter::ToggleInventoryWidget);
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
		struct LazyUpdatePair
		{
			USceneComponent* OldParent;
			USceneComponent* OldChild;
			USceneComponent* NewParent;
			USceneComponent* NewChild;
		};

		TSet<USceneComponent*> Initialized;
		TArray<LazyUpdatePair> LazyUpdates;
		
		// 블루프린트의 컴포넌트 속성 복사
		for ( TFieldIterator<FObjectProperty> It(GetClass()); It; ++It  )
		{
			if ( It->PropertyClass->IsChildOf( UActorComponent::StaticClass() ) &&
				 !It->PropertyClass->IsChildOf( UInputComponent::StaticClass() ) )
			{
				UActorComponent* ThisComponent = Cast<UActorComponent>( It->GetObjectPropertyValue_InContainer( this ) );
				UActorComponent* OriginComponent = Cast<UActorComponent>( It->GetObjectPropertyValue_InContainer( DefaultAsset ) );

				if ( !ThisComponent || !OriginComponent )
				{
					continue;
				}
				
				// 프로퍼티 복사 후 컴포넌트 재등록하면서 갱신
				ThisComponent->UnregisterComponent();
				
				if ( IsValid( GEngine ) )
				{
					UEngine::FCopyPropertiesForUnrelatedObjectsParams Params{};
					Params.bDoDelta = true;
					Params.bSkipCompilerGeneratedDefaults = true;
					Params.bReplaceInternalReferenceUponRead = true;
					std::remove_pointer_t<decltype(Params.OptionalReplacementMappings)> TempReplacementMappings;
					Params.OptionalReplacementMappings = &TempReplacementMappings;

					// Scene Component이면...
					if ( USceneComponent* OriginSceneComponent = Cast<USceneComponent>( OriginComponent ) )
					{
						// 부모로 부착된 컴포넌트가 있는지 확인하고...
						if ( USceneComponent* OriginParentComponent = OriginSceneComponent->GetAttachParent() )
						{
							// 똑같은 컴포넌트를 프로퍼티로 찾아서...
							for ( TFieldIterator<FObjectProperty> ParentIt( GetClass() ); ParentIt; ++ParentIt )
							{
								if ( ParentIt->PropertyClass->IsChildOf( USceneComponent::StaticClass() ) )
								{
									USceneComponent* ComponentPtrFromOrigin = Cast<USceneComponent>( ParentIt->GetObjectPropertyValue_InContainer( DefaultAsset ) );

									// 이 객체의 프로퍼티에 있는 컴포넌트에 똑같이 적용
									if ( ComponentPtrFromOrigin == OriginParentComponent )
									{
										USceneComponent* ThisParentComponent = Cast<USceneComponent>( ParentIt->GetObjectPropertyValue_InContainer( this ) );
										USceneComponent* ThisSceneComponent = Cast<USceneComponent>( ThisComponent );
										
										TempReplacementMappings.Emplace( OriginParentComponent, ThisParentComponent );
									
										if ( ThisParentComponent && Initialized.Contains( ThisParentComponent ) )
										{
											ThisSceneComponent->AttachToComponent( ThisParentComponent, FAttachmentTransformRules::KeepRelativeTransform, OriginSceneComponent->GetAttachSocketName() );
											Initialized.Emplace( Cast<USceneComponent>( ThisComponent ) );
										}
										else
										{
											// 일시적으로 부착을 풀고
											ThisSceneComponent->DetachFromComponent( FDetachmentTransformRules::KeepRelativeTransform );
											LazyUpdates.Emplace( OriginParentComponent, OriginSceneComponent, ThisParentComponent, ThisSceneComponent );
										}
										break;
									}
								}
							}
						}
						else
						{
							// 부모가 없다면 그대로 초기화 판정
							Initialized.Emplace( Cast<USceneComponent>( ThisComponent ) );
						}
					}
					
					GEngine->CopyPropertiesForUnrelatedObjects( OriginComponent, ThisComponent, Params );
				}
				
				ThisComponent->RegisterComponent();
			}
		}

		// 부모가 초기화가 안된 상태에서 자식을 붙이려 한 경우에 대해 게으른 초기화
		for ( auto SetIt = LazyUpdates.CreateIterator(); SetIt; ++SetIt )
		{
			const auto& [Old, OldChild, This, ThisChild] = *SetIt;
			if ( Initialized.Contains( This ) )
			{
				ThisChild->AttachToComponent( This, FAttachmentTransformRules::KeepRelativeTransform, OldChild->GetAttachSocketName() );
				Initialized.Emplace( ThisChild );
				SetIt.RemoveCurrentSwap();
			}
		}

		check( LazyUpdates.IsEmpty() );

		FObjectPropertyUtility::CopyClassPropertyIfTypeEquals<ANACharacter, UInputMappingContext, UInputAction>( this, DefaultAsset );

		ApplyAttachments();
		const FTransform Transform = DefaultAsset->GetMesh()->GetRelativeTransform();
		
		GetMesh()->SetRelativeTransform(Transform);
		// 리플리케이션을 위한 Mesh Offset
		CacheInitialMeshOffset( Transform.GetLocation(), Transform.Rotator() );
	}
}

void ANACharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	GetAbilitySystemComponent()->InitAbilityActorInfo
	(
		GetPlayerState<ANAPlayerState>(),
		this
	);
}

void ANACharacter::OnConstruction( const FTransform& Transform )
{
	Super::OnConstruction( Transform );

#if WITH_EDITOR
	if ( GetWorld()->IsEditorWorld() )
	{
		ApplyAttachments();	
	}
#endif
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

void ANACharacter::StartLeftMouseAttack()
{
	if ( !TryAttack( RightHandChildActor->GetChildActor() ) )
	{
		if ( DefaultCombatComponent->IsAbleToAttack() && !DefaultCombatComponent->IsAttacking() )
		{
			DefaultCombatComponent->StartAttack();	
		}
	}
}

void ANACharacter::StopLeftMouseAttack()
{
	if ( !TryStopAttack( RightHandChildActor->GetChildActor() ) )
	{
		if ( DefaultCombatComponent->IsAttacking() )
		{
			DefaultCombatComponent->StopAttack();	
		}
	}
}

void ANACharacter::TryInteract()
{
	if (ensure(InteractionComponent != nullptr))
	{
		InteractionComponent->BeginInteraction();
	}
}

void ANACharacter::ToggleInventoryWidget()
{
	if (ensure(InventoryComponent != nullptr))
	{
		if (InventoryComponent->IsWidgetVisible())
		{
			InventoryComponent->CollapseInventoryWidget();
		}
		else
		{
			InventoryComponent->ReleaseInventoryWidget();
		}
	}
}

void ANACharacter::TryRevive()
{
	// todo: GAS의 Input 감지를 이용할 수 있음
	if ( AbilitySystemComponent )
	{
		AbilitySystemComponent->AbilityLocalInputPressed( static_cast<int32>( EAbilityInputID::Revive ) );
	}
}

void ANACharacter::StopRevive()
{
	// todo: GAS의 Input 감지를 이용할 수 있음
	if ( AbilitySystemComponent )
	{
		AbilitySystemComponent->AbilityLocalInputReleased( static_cast<int32>( EAbilityInputID::Revive ) );
	}
}

void ANACharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME( ANACharacter, AbilitySystemComponent );
	DOREPLIFETIME( ANACharacter, DefaultCombatComponent );
	DOREPLIFETIME( ANACharacter, LeftHandChildActor );
	DOREPLIFETIME( ANACharacter, RightHandChildActor );
}
