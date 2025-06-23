// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/WidgetComponent/NAAmmoIndicatorComponent.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "ARPG/ARPG.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "Components/TextBlock.h"
#include "Item/PickableItem//NAWeapon.h"
#include "Weapon/Widget/NAAmmoWidget.h"

FLinearColor UNAAmmoIndicatorComponent::EnoughColor = {0.322917, 0.75036, 1.0, 0.5};
FLinearColor UNAAmmoIndicatorComponent::LowColor = {1.0, 0.75036f, 0.322917, 0.5};


// Sets default values for this component's properties
UNAAmmoIndicatorComponent::UNAAmmoIndicatorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

	FIND_CLASS( WidgetClass, "/Script/UMGEditor.WidgetBlueprint'/Game/00_ProjectNA/01_Blueprint/01_Widget/InGame/BP_NAAmmoWidget.BP_NAAmmoWidget_C'")
	FIND_OBJECT( FrameMaterial,  "/Script/Engine.Material'/Game/00_ProjectNA/05_Resource/01_Material/Widgets/M_AmmoIndicator.M_AmmoIndicator'")
	FIND_OBJECT( BackgroundMaterial, "/Script/Engine.Material'/Game/00_ProjectNA/05_Resource/01_Material/Widgets/M_AmmoIndicator_Background.M_AmmoIndicator_Background'")

	SetWidgetSpace( EWidgetSpace::World );
}

void UNAAmmoIndicatorComponent::UpdateAmmoCount( FActiveGameplayEffectHandle ActiveGameplayEffectHandle, int I,
	int Arg )
{
	UpdateAmmoCount( I );	
}

FText UNAAmmoIndicatorComponent::GetAmmoCount()
{
	return FText::FromString( FString::FromInt( CurrentAmmo ) );
}

void UNAAmmoIndicatorComponent::UpdateAmmoCount( const int32 Count )
{
	CurrentAmmo = Count;
	static constexpr int32 LowThreshold = 5;
	
	InstancedBackgroundMaterial->SetScalarParameterValue( "Low", Count <= LowThreshold );
	InstancedFrameMaterial->SetScalarParameterValue( "Low", Count <= LowThreshold );
		
	if ( UNAAmmoWidget* AmmoWidget = Cast<UNAAmmoWidget>( GetWidget() ) )
	{
		UTextBlock* TextBlock = AmmoWidget->GetAmmoCountText();
		check( TextBlock );

		if ( TextBlock )
		{
			const FLinearColor ColorToUpdate = Count <= LowThreshold ? LowColor : EnoughColor;
			TextBlock->SetColorAndOpacity( ColorToUpdate );
			TextBlock->TextDelegate.BindDynamic( this, &UNAAmmoIndicatorComponent::GetAmmoCount );
			TextBlock->SynchronizeProperties();
		}
	}
}

void UNAAmmoIndicatorComponent::InitializeWidgetMaterial( UMaterialInstanceDynamic* Frame, UMaterialInstanceDynamic* Background ) const
{
	const UNAAmmoWidget* AmmoWidget = Cast<UNAAmmoWidget>( GetWidget() );
	check( AmmoWidget );
	if ( AmmoWidget )
	{
		if ( Frame && Background )
		{
			AmmoWidget->SetBackgroundHologram( Background );
			AmmoWidget->SetHologramFrame( Frame, Frame );
		}
	}
}

void UNAAmmoIndicatorComponent::HandleAmmoAdded( UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle )
{
	UNAMontageCombatComponent* CombatComponent = GetOwner()->GetComponentByClass<UNAMontageCombatComponent>();
	check( CombatComponent );

	if ( CombatComponent )
	{
		if ( GameplayEffectSpec.Def->IsA( CombatComponent->GetAmmoType() ) )
		{
			AbilitySystemComponent->OnGameplayEffectStackChangeDelegate( ActiveGameplayEffectHandle )->AddUObject( this, &UNAAmmoIndicatorComponent::UpdateAmmoCount );
			const int32 InitialAmmoCount = AbilitySystemComponent->GetCurrentStackCount( ActiveGameplayEffectHandle );
			UpdateAmmoCount( InitialAmmoCount );
			TrackingAmmoHandle = ActiveGameplayEffectHandle;
		}	
	}
}

void UNAAmmoIndicatorComponent::CleanupAmmoAdded( const FActiveGameplayEffect& ActiveGameplayEffect )
{
	const UNAMontageCombatComponent* CombatComponent = GetOwner()->GetComponentByClass<UNAMontageCombatComponent>();
	check( CombatComponent );

	if ( ActiveGameplayEffect.Spec.Def->IsA( CombatComponent->GetAmmoType() ) )
	{
		UpdateAmmoCount( 0 );
		TrackingAmmoHandle = {};
	}
}

AActor* UNAAmmoIndicatorComponent::GetAttachedParentActor() const
{
	if ( AActor* AttachedParent = GetOwner()->GetAttachParentActor() )
	{
		return AttachedParent;
	}

	if ( const USceneComponent* AttachedComponent = GetOwner()->GetParentComponent() )
	{
		return AttachedComponent->GetOwner();
	}

	return nullptr;
}


// Called when the game starts
void UNAAmmoIndicatorComponent::BeginPlay()
{
	Super::BeginPlay();

	bool HasParent = false;
	if ( const APawn* OwningPawn = Cast<APawn>( GetAttachedParentActor() ) )
	{
		if ( OwningPawn->IsLocallyControlled() )
		{
			// 총알 위젯은 실제 캐릭터가 장착했을때만 뜨도록
			HasParent = true;
		}
	}
	SetVisibility( HasParent );
	
	// ...

	if ( FrameMaterial )
	{
		InstancedFrameMaterial = UMaterialInstanceDynamic::Create( FrameMaterial, this );
	}
	if ( BackgroundMaterial )
	{
		InstancedBackgroundMaterial = UMaterialInstanceDynamic::Create( BackgroundMaterial, this );
	}
			
	InitializeWidgetMaterial( InstancedFrameMaterial, InstancedBackgroundMaterial );

	if ( HasParent )
	{
		if ( UNAAmmoWidget* AmmoWidget = Cast<UNAAmmoWidget>( GetWidget() ) )
		{
			UTextBlock* TextBlock = AmmoWidget->GetAmmoCountText();
			check( TextBlock );

			if ( TextBlock )
			{
				TextBlock->TextDelegate.BindDynamic( this, &UNAAmmoIndicatorComponent::GetAmmoCount );
				TextBlock->SynchronizeProperties();
			}
		}

		if ( const TScriptInterface<IAbilitySystemInterface>& Interface = GetAttachedParentActor() )
		{
			if ( const ANAWeapon* Weapon = Cast<ANAWeapon>( GetOwner() ) )
			{
				Interface->GetAbilitySystemComponent()->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject( this, &UNAAmmoIndicatorComponent::HandleAmmoAdded );
				Interface->GetAbilitySystemComponent()->OnAnyGameplayEffectRemovedDelegate().AddUObject( this, &UNAAmmoIndicatorComponent::CleanupAmmoAdded );
			
				const UNAMontageCombatComponent* CombatComponent = Weapon->GetComponentByClass<UNAMontageCombatComponent>();
				ensure( CombatComponent );
				const TSubclassOf<UGameplayEffect> AmmoType = CombatComponent->GetAmmoType();
				ensure( AmmoType );

				const FActiveGameplayEffectsContainer& Container = Interface->GetAbilitySystemComponent()->GetActiveGameplayEffects();
				const TArray<FActiveGameplayEffectHandle>& Array = Container.GetAllActiveEffectHandles();

				for ( auto It = Array.CreateConstIterator(); It; ++It )
				{
					const FActiveGameplayEffect* Effect = Container.GetActiveGameplayEffect( *It );
					if ( Effect->Spec.Def->IsA( AmmoType ) )
					{
						TrackingAmmoHandle = *It;
						break;
					}
				}
			
				if ( TrackingAmmoHandle.IsValid() )
				{
					CurrentAmmo = Interface->GetAbilitySystemComponent()->GetCurrentStackCount( TrackingAmmoHandle );
					Interface->GetAbilitySystemComponent()->OnGameplayEffectStackChangeDelegate( TrackingAmmoHandle )->AddUObject( this, &UNAAmmoIndicatorComponent::UpdateAmmoCount );
				}
			
				UpdateAmmoCount( CurrentAmmo );
			}
		}
	}
}

void UNAAmmoIndicatorComponent::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
	Super::EndPlay( EndPlayReason );

	if ( const TScriptInterface<IAbilitySystemInterface>& Interface = GetAttachedParentActor() )
	{
		Interface->GetAbilitySystemComponent()->OnActiveGameplayEffectAddedDelegateToSelf.RemoveAll( this );
		Interface->GetAbilitySystemComponent()->OnAnyGameplayEffectRemovedDelegate().RemoveAll( this );
		if ( TrackingAmmoHandle.IsValid() )
		{
			Interface->GetAbilitySystemComponent()->OnGameplayEffectStackChangeDelegate( TrackingAmmoHandle )->RemoveAll( this );	
		}
	}
}

// Called every frame
void UNAAmmoIndicatorComponent::TickComponent( float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

