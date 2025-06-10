// Fill out your copyright notice in the Description page of Project Settings.


#include "HP/WidgetComponent/NAReviveWidgetComponent.h"

#include "AbilitySystemComponent.h"
#include "NACharacter.h"
#include "GameFramework/GameStateBase.h"
#include "HP/GameplayEffect/NAGE_Revive.h"
#include "HP/Widget/NAReviveWidget.h"

DEFINE_LOG_CATEGORY( LogReviveWidget );


// Sets default values for this component's properties
UNAReviveWidgetComponent::UNAReviveWidgetComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// ...
	static ConstructorHelpers::FObjectFinder<UMaterial> M_CircularProgress( TEXT("/Script/Engine.Material'/Game/00_ProjectNA/05_Resource/01_Material/Widgets/M_CircularProgress.M_CircularProgress'") );
	if ( M_CircularProgress.Succeeded() )
	{
		BaseMaterial = M_CircularProgress.Object;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> WG_Revive( TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/00_ProjectNA/01_Blueprint/01_Widget/InGame/BP_NAReviveWidget.BP_NAReviveWidget_C'") );
	if ( WG_Revive.Succeeded() )
	{
		SetWidgetClass( WG_Revive.Class );
	}
}

void UNAReviveWidgetComponent::OnReviveApplied( UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle )
{
	if ( GameplayEffectSpec.Def->IsA<UNAGE_Revive>() )
	{
		UE_LOG( LogReviveWidget, Log, TEXT("%hs: Revive Applied caught"), __FUNCTION__ );
		RuntimeInstance->SetScalarParameterValue( TEXT("Progress"), 0.f );

		const FActiveGameplayEffect* Context = AbilitySystemComponent->GetActiveGameplayEffect( ActiveGameplayEffectHandle );
		StartInServerTime = Context->StartServerWorldTime;
		
		SetVisibility( true );
		if ( !GetUserWidgetObject() )
		{
			InitWidget(); // todo: 왜 이 시점에서 위젯이 없지?
			UE_LOG( LogReviveWidget, Warning, TEXT( "%hs: %s not initialized, Reinitializing... " ), __FUNCTION__, *GetNameSafe( GetUserWidgetObject() ) )
		}
		if ( UNAReviveWidget* ReviveWidget = Cast<UNAReviveWidget>( GetUserWidgetObject() ) )
		{
			ReviveWidget->SetImage( RuntimeInstance );
		}
		SetComponentTickEnabled( true );
	}
}

void UNAReviveWidgetComponent::OnReviveRemoved( const FActiveGameplayEffect& ActiveGameplayEffect )
{
	if ( ActiveGameplayEffect.Spec.Def->IsA<UNAGE_Revive>() )
	{
		UE_LOG( LogReviveWidget, Log, TEXT("%hs: Revive Removed caught"), __FUNCTION__ );
		RuntimeInstance->SetScalarParameterValue( TEXT("Progress"), 0.f );
		StartInServerTime = std::numeric_limits<double>::max();
		
		SetVisibility( false );
		SetComponentTickEnabled( false );
	}
}

// Called when the game starts
void UNAReviveWidgetComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	RuntimeInstance = UMaterialInstanceDynamic::Create( BaseMaterial, this );

	InitWidget();
	if ( UNAReviveWidget* ReviveWidget = Cast<UNAReviveWidget>( GetUserWidgetObject() ) )
	{
		ReviveWidget->SetImage( RuntimeInstance );
		ReviveWidget->SetVisibility( ESlateVisibility::Visible );
	}
	
	SetDrawSize( { 100, 100 } );
	SetAbsolute( false, true, false );
	SetVisibility( false );
	SetComponentTickEnabled( false );

	const ANACharacter* Character = Cast<ANACharacter>( GetOwner() );
	check( Character );
	
	Character->GetAbilitySystemComponent()->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject( this, &UNAReviveWidgetComponent::OnReviveApplied );
	Character->GetAbilitySystemComponent()->OnAnyGameplayEffectRemovedDelegate().AddUObject( this, &UNAReviveWidgetComponent::OnReviveRemoved );
}


// Called every frame
void UNAReviveWidgetComponent::TickComponent( float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...

	if ( const AGameStateBase* GameState = GetWorld()->GetGameState() )
	{
		const float Ratio = 1.f - ((GameState->GetServerWorldTimeSeconds() - StartInServerTime) / 5.f);
		RuntimeInstance->SetScalarParameterValue( TEXT("Progress"), FMath::Clamp( Ratio, 0.f, 1.f ) );
	}
	
	if ( const APlayerController* PlayerController = Cast<APlayerController>( GetWorld()->GetFirstPlayerController() ) )
	{
		const APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
		const FVector CameraLocation = CameraManager->GetCameraLocation();
		const FVector WidgetLocation = GetComponentLocation();

		const FVector Direction = CameraLocation - WidgetLocation;
		const FRotator Rotation = FRotationMatrix::MakeFromX(Direction).Rotator();
		SetRelativeRotation( Rotation );
	}
}

