
#include "Item/ItemActor/NAItemActor.h"

#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Interaction/NAInteractionComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "Item/ItemWidget/NAItemWidgetComponent.h"


ANAItemActor::ANAItemActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ItemCollision = CreateOptionalDefaultSubobject<USphereComponent>(TEXT("ItemCollision(Sphere)"));
	ItemMesh = CreateOptionalDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh(Static)"));

	if ( ItemCollision )
	{
		bWasItemCollisionCreated = true;
		SetRootComponent(ItemCollision);
	}
	if ( ItemMesh )
	{
		bWasItemMeshCreated = true;
	}
	
	TriggerSphere = CreateDefaultSubobject<USphereComponent>("TriggerSphere");
	TriggerSphere->SetRelativeLocation(FVector(0.f, 0.f, 140.f));
	TriggerSphere->SetSphereRadius(180.0f);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->CanCharacterStepUpOn = ECB_No;
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); 
	TriggerSphere->SetSimulatePhysics(false);
	
	ItemWidgetComponent = CreateDefaultSubobject<UNAItemWidgetComponent>(TEXT("ItemWidgetComponent"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface>
		ItemWidgetMaterial(TEXT(
			"/Script/Engine.MaterialInstanceConstant'/Engine/EngineMaterials/Widget3DPassThrough_Translucent.Widget3DPassThrough_Translucent'"));
	check(ItemWidgetMaterial.Object);
	ItemWidgetComponent->SetMaterial(0, ItemWidgetMaterial.Object);
	
	ItemDataID = NAME_None;
	
	SetReplicates( true );
	SetReplicateMovement( true );
	bAlwaysRelevant = true;
}

void ANAItemActor::PostInitProperties()
{
	Super::PostInitProperties();

	//ensureAlways(ItemCollision && GetRootComponent() == ItemCollision);
}

void ANAItemActor::PostReinitProperties()
{
	Super::PostReinitProperties();
}

void ANAItemActor::PostLoad()
{
	Super::PostLoad();
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		if (ItemDataID.IsNone() && !GetWorld()->IsPreviewWorld()
			&& !IsChildActor())
		{
			InitItemData();
		}
	}
}

void ANAItemActor::PostActorCreated()
{
	Super::PostActorCreated();

	// ChildActorComponent에 의해 생성된 경우
	if (IsChildActor())
	{
		if (ItemCollision)
		{
			ItemCollision->SetSimulatePhysics(false);
			ItemCollision->SetGenerateOverlapEvents(false);
			ItemCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			ItemCollision->Deactivate();
		}
		if (TriggerSphere)
		{
			TriggerSphere->SetGenerateOverlapEvents(false);
			TriggerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			TriggerSphere->Deactivate();
		}
		if (ItemWidgetComponent)
		{
			ItemWidgetComponent->SetVisibility(false);
			ItemWidgetComponent->Deactivate();
		}
		if (ItemMesh)
		{
			ItemMesh->SetCollisionEnabled( ECollisionEnabled::NoCollision );
			ItemMesh->SetSimulatePhysics( false );
			ItemMesh->SetGenerateOverlapEvents(false);
		}
	}
}

#if WITH_EDITOR
void ANAItemActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

EItemSubobjDirtyFlags ANAItemActor::CheckDirtySubobjectFlags(const FNAItemBaseTableRow* MetaData) const
{
	EItemSubobjDirtyFlags DirtyFlags = EItemSubobjDirtyFlags::ISDF_None;
	
	UClass* ItemClass = GetClass();
	if (!ItemClass)
	{
		ensure(false);
		return DirtyFlags;
	}
	if (!MetaData)
	{
		ensure(false);
		return DirtyFlags;
	}
	
	if (MetaData->CollisionShape != EItemCollisionShape::ICS_None && bWasItemCollisionCreated)
	{
		if (!ItemCollision)
		{
			EnumAddFlags(DirtyFlags, EItemSubobjDirtyFlags::ISDF_CollisionShape);
		}
		else
		{
			switch (MetaData->CollisionShape)
			{
			case EItemCollisionShape::ICS_Sphere:
				if (!ItemCollision->IsA<USphereComponent>())
				{
					EnumAddFlags(DirtyFlags, EItemSubobjDirtyFlags::ISDF_CollisionShape);
				}
				break;
		
			case EItemCollisionShape::ICS_Box:
				if (!ItemCollision->IsA<UBoxComponent>())
				{
					EnumAddFlags(DirtyFlags, EItemSubobjDirtyFlags::ISDF_CollisionShape);
				}
				break;
		
			case EItemCollisionShape::ICS_Capsule:
				if (!ItemCollision->IsA<UCapsuleComponent>())
				{
					EnumAddFlags(DirtyFlags, EItemSubobjDirtyFlags::ISDF_CollisionShape);
				}
				break;
		
			default:
				ensure(false);
				break;
			}
		}
	}
	
	if (MetaData->MeshType != EItemMeshType::IMT_None && bWasItemMeshCreated)
	{
		if (!ItemMesh)
		{
			EnumAddFlags(DirtyFlags,EItemSubobjDirtyFlags::ISDF_MeshType);
		}
		else
		{
			switch (MetaData->MeshType)
			{
			case EItemMeshType::IMT_Static:
				if (!ItemMesh->IsA<UStaticMeshComponent>())
				{
					EnumAddFlags(DirtyFlags,EItemSubobjDirtyFlags::ISDF_MeshType);
				}
				break;
		
			case EItemMeshType::IMT_Skeletal:
				if (!ItemMesh->IsA<USkeletalMeshComponent>())
				{
					EnumAddFlags(DirtyFlags,EItemSubobjDirtyFlags::ISDF_MeshType);
				}
				break;
		
			default:
				ensure(false);
				break;
			}
		}
	}

	return DirtyFlags;
}

void ANAItemActor::OnConstruction(const FTransform& Transform)
{
 	Super::OnConstruction(Transform);
 	
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}
	
	if (!UNAItemEngineSubsystem::Get()
		|| !UNAItemEngineSubsystem::Get()->IsItemMetaDataInitialized()
#if WITH_EDITOR
		|| !UNAItemEngineSubsystem::Get()->IsRegisteredItemMetaClass(GetClass()))
#endif
	{
		return;
	}
	
	if (ItemDataID.IsNone() && !GetWorld()->IsPreviewWorld()
		&& !IsChildActor()) // ChildActorComponent에 의해 생성된 경우: 아이템 데이터 새로 생성 x
	{
		InitItemData();
	}

	const FNAItemBaseTableRow* MetaData = UNAItemEngineSubsystem::Get()->GetItemMetaDataByClass(GetClass());
	if (!MetaData) { return; }
	
	EItemSubobjDirtyFlags DirtyFlags = CheckDirtySubobjectFlags(MetaData);
	const FTransform PreviousTransform = RootComponent->GetComponentTransform();
	
	if (DirtyFlags != EItemSubobjDirtyFlags::ISDF_None)
	{
		EObjectFlags SubobjFlags = GetMaskedFlags(RF_PropagateToSubObjects);
		
		TArray<UActorComponent*> OldComponents;
		OldComponents.Reset();
		
		if (EnumHasAnyFlags(DirtyFlags, EItemSubobjDirtyFlags::ISDF_CollisionShape))
		{
			if (IsValid(ItemCollision))
			{
				OldComponents.Emplace(ItemCollision);
			}
			
			switch (MetaData->CollisionShape)
			{
			case EItemCollisionShape::ICS_Sphere:
				ItemCollision = NewObject<USphereComponent>(this, TEXT("ItemCollision(Sphere)"), SubobjFlags);
				break;
			case EItemCollisionShape::ICS_Box:
				ItemCollision = NewObject<UBoxComponent>(this, TEXT("ItemCollision(Box)"), SubobjFlags);
				break;
			case EItemCollisionShape::ICS_Capsule:
				ItemCollision = NewObject<UCapsuleComponent>(this, TEXT("ItemCollision(Capsule)"), SubobjFlags);
				break;
			default:
				ensure(false);
				break;
			}
			ensure(ItemCollision != nullptr);
		}
		
		if (EnumHasAnyFlags(DirtyFlags, EItemSubobjDirtyFlags::ISDF_MeshType))
		{
			if (IsValid(ItemMesh))
			{
				OldComponents.Emplace(ItemMesh);
			}

			switch (MetaData->MeshType)
			{
			case EItemMeshType::IMT_Static:
				ItemMesh = NewObject<UStaticMeshComponent>(this, TEXT("ItemMesh(Static)"), SubobjFlags);
				break;
			case EItemMeshType::IMT_Skeletal:
				ItemMesh = NewObject<USkeletalMeshComponent>(this, TEXT("ItemMesh(Skeletal)"), SubobjFlags);
				break;
			default:
				ensure(false);
				break;
			}

			ensure(ItemMesh != nullptr);
		}
		
		if (!OldComponents.IsEmpty())
		{
			for (UActorComponent* OldComponent : OldComponents)
			{
				if ( const USceneComponent* SceneComponent = Cast<USceneComponent>( OldComponent ) )
				{
					const TArray<USceneComponent*>& ChildComponents = SceneComponent->GetAttachChildren();
					for ( auto It = ChildComponents.CreateConstIterator(); It; ++It )
					{
						(*It)->DetachFromComponent( FDetachmentTransformRules::KeepRelativeTransform );
					}
				}
				
				OldComponent->ClearFlags(RF_Standalone | RF_Public);
				OldComponent->DestroyComponent();
				RemoveInstanceComponent(OldComponent);
			}
			OldComponents.Empty();
		}
	}
	
	// 어태치먼트
	if (ItemCollision && GetRootComponent() != ItemCollision)
	{
		if (USceneComponent* OldRoot = GetRootComponent())
		{
			const TArray<USceneComponent*>& ChildComponents = OldRoot->GetAttachChildren();
			for ( auto It = ChildComponents.CreateConstIterator(); It; ++It )
			{
				(*It)->DetachFromComponent( FDetachmentTransformRules::KeepRelativeTransform );
			}
			OldRoot->DestroyComponent();
			RemoveInstanceComponent(OldRoot);
		}
		SetRootComponent(ItemCollision);
	}
	if (ItemMesh &&  ItemMesh->GetAttachParent() != ItemCollision)
	{
		ItemMesh->AttachToComponent(ItemCollision, FAttachmentTransformRules::KeepRelativeTransform);
	}
	if (TriggerSphere && TriggerSphere->GetAttachParent() != ItemCollision)
	{
		TriggerSphere->AttachToComponent(ItemCollision, FAttachmentTransformRules::KeepRelativeTransform);
	}
	if (ItemWidgetComponent && ItemWidgetComponent->GetAttachParent() != ItemCollision)
	{
		ItemWidgetComponent->AttachToComponent(ItemCollision, FAttachmentTransformRules::KeepRelativeTransform);
	}

	// 부모, 자식에서 Property로 설정된 컴포넌트들을 조회
	TSet<UActorComponent*> SubObjsActorComponents;
	for ( TFieldIterator<FObjectProperty> It ( GetClass() ); It; ++It )
	{
		if ( It->PropertyClass->IsChildOf( UActorComponent::StaticClass() ) )
		{
			if ( UActorComponent* Component = Cast<UActorComponent>( It->GetObjectPropertyValue_InContainer( this ) ) )
			{
				SubObjsActorComponents.Add( Component );
			}
		}
	}
	
	for (UActorComponent* OwnedComponent : GetComponents().Array())
	{
		if (USceneComponent* OwnedSceneComp = Cast<USceneComponent>(OwnedComponent))
		{
			if ( SubObjsActorComponents.Contains( OwnedComponent ) )
			{
				if (!OwnedSceneComp->IsRegistered())
				{
					OwnedSceneComp->RegisterComponent();
				}
				continue;
			}

			TArray<USceneComponent*> AttachedChildren = OwnedSceneComp->GetAttachChildren();
			for (USceneComponent* Child : AttachedChildren)
			{
				if (IsValid(Child))
				{
					Child->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
				}
			}
			AttachedChildren.Empty();

			OwnedSceneComp->ClearFlags(RF_Standalone | RF_Public);
			OwnedSceneComp->DestroyComponent();
			RemoveInstanceComponent(OwnedSceneComp);
		}
	}

	if (MetaData->CollisionShape != EItemCollisionShape::ICS_None)
	{
		if (USphereComponent* SphereCollision = Cast<USphereComponent>(ItemCollision))
		{
			SphereCollision->SetSphereRadius(MetaData->CollisionSphereRadius);
		}
		else if (UBoxComponent* BoxCollision = Cast<UBoxComponent>(ItemCollision))
		{
			BoxCollision->SetBoxExtent(MetaData->CollisionBoxExtent);
		}
		else if (UCapsuleComponent* CapsuleCollision = Cast<UCapsuleComponent>(ItemCollision))
		{
			CapsuleCollision->SetCapsuleSize(MetaData->CollisionCapsuleSize.X, MetaData->CollisionCapsuleSize.Y);
		}
	}

	if (MetaData->MeshType != EItemMeshType::IMT_None)
	{
		if (ItemMesh)
		{
			ItemMesh->SetRelativeTransform(MetaData->MeshTransform);
		}
		
		if (UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(ItemMesh))
		{
			StaticMeshComp->SetStaticMesh(MetaData->StaticMeshAssetData.StaticMesh);
			ItemFractureCollection = MetaData->StaticMeshAssetData.FractureCollection;
			ItemFractureCache =  MetaData->StaticMeshAssetData.FractureCache;
		}
		else if (USkeletalMeshComponent* SkeletalMeshComp = Cast<USkeletalMeshComponent>(ItemMesh))
		{
			SkeletalMeshComp->SetSkeletalMesh(MetaData->SkeletalMeshAssetData.SkeletalMesh);
			SkeletalMeshComp->SetAnimClass(MetaData->SkeletalMeshAssetData.AnimClass);
		}
	}

	// 트랜스폼 및 콜리전, 피직스 등등 설정 여기에
	if (ItemCollision)
	{
		ItemCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ItemCollision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		ItemCollision->SetSimulatePhysics( true );
		ItemCollision->SetIsReplicated( true );
	}
	if ( ItemMesh )
	{
		ItemMesh->SetCollisionEnabled( ECollisionEnabled::NoCollision );
		ItemMesh->SetSimulatePhysics( false );
		ItemMesh->SetGenerateOverlapEvents(false);
	}
	
	GetRootComponent()->SetWorldTransform(PreviousTransform);
}

void ANAItemActor::Destroyed()
{
	Super::Destroyed();

	if (!HasActorBegunPlay()) return;
	if (ItemWidgetComponent && ItemWidgetComponent->IsVisible())
	{
		ItemWidgetComponent->CollapseItemWidgetPopup();
	}
}

void ANAItemActor::InitItemData()
{
	if (HasValidItemID())
	{
		return;
	}
	
	if (const UNAItemData* NewItemData = UNAItemEngineSubsystem::Get()->CreateItemDataByActor(this))
	{
		ItemDataID = NewItemData->GetItemID();
		OnItemDataInitialized();
	}
}

void ANAItemActor::OnItemDataInitialized()
{
	VerifyInteractableData();
}

void ANAItemActor::VerifyInteractableData()
{
	if (InteractableInterfaceRef != nullptr)
	{
		return;
	}
	
	// 이 액터가 UNAInteractableInterface 인터페이스를 구현했다면 this를 할당
	if (HasValidItemID() && GetClass()->ImplementsInterface(UNAInteractableInterface::StaticClass()))
	{
		InteractableInterfaceRef = this;
	}
	else
	{
		ensureAlways(false);
	}
}

void ANAItemActor::OnActorBeginOverlap_Impl(AActor* OverlappedActor, AActor* OtherActor)
{
	Execute_NotifyInteractableFocusBegin(this, OverlappedActor, OtherActor);
}

void ANAItemActor::OnActorEndOverlap_Impl(AActor* OverlappedActor, AActor* OtherActor)
{
	Execute_NotifyInteractableFocusEnd(this, OverlappedActor, OtherActor);
}

void ANAItemActor::BeginPlay()
{
	/**
	 * @TODO: 플레이 때 서브오브젝트 계층구조 무너지면 이 부분 활성화 하시오
	 */
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const FTransform PreviousTransform = RootComponent->GetComponentTransform();
	
	if (ItemCollision && GetRootComponent() != ItemCollision)
	{
		if (USceneComponent* OldRoot = GetRootComponent())
		{
			const TArray<USceneComponent*>& ChildComponents = OldRoot->GetAttachChildren();
			for ( auto It = ChildComponents.CreateConstIterator(); It; ++It )
			{
				(*It)->DetachFromComponent( FDetachmentTransformRules::KeepRelativeTransform );
			}
			OldRoot->DestroyComponent();
			RemoveInstanceComponent(OldRoot);
		}
		SetRootComponent(ItemCollision);
	}
	if (ItemMesh &&  ItemMesh->GetAttachParent() != ItemCollision)
	{
		ItemMesh->AttachToComponent(ItemCollision, FAttachmentTransformRules::KeepRelativeTransform);
	}
	if (TriggerSphere && TriggerSphere->GetAttachParent() != ItemCollision)
	{
		TriggerSphere->AttachToComponent(ItemCollision, FAttachmentTransformRules::KeepRelativeTransform);
	}
	if (ItemWidgetComponent && ItemWidgetComponent->GetAttachParent() != ItemCollision)
	{
		ItemWidgetComponent->AttachToComponent(ItemCollision, FAttachmentTransformRules::KeepRelativeTransform);
	}

	// 부모, 자식에서 Property로 설정된 컴포넌트들을 조회
	TSet<UActorComponent*> SubObjsActorComponents;
	for ( TFieldIterator<FObjectProperty> It ( GetClass() ); It; ++It )
	{
		if ( It->PropertyClass->IsChildOf( UActorComponent::StaticClass() ) )
		{
			if ( UActorComponent* Component = Cast<UActorComponent>( It->GetObjectPropertyValue_InContainer( this ) ) )
			{
				SubObjsActorComponents.Add( Component );
			}
		}
	}
	
	for (UActorComponent* OwnedComponent : GetComponents().Array())
	{
		if (USceneComponent* OwnedSceneComp = Cast<USceneComponent>(OwnedComponent))
		{
			if ( SubObjsActorComponents.Contains( OwnedComponent ) )
			{
				if (!OwnedSceneComp->IsRegistered())
				{
					OwnedSceneComp->RegisterComponent();
				}
				continue;
			}

			TArray<USceneComponent*> AttachedChildren = OwnedSceneComp->GetAttachChildren();
			for (USceneComponent* Child : AttachedChildren)
			{
				if (IsValid(Child))
				{
					Child->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
				}
			}
			AttachedChildren.Empty();

			OwnedSceneComp->ClearFlags(RF_Standalone | RF_Public);
			OwnedSceneComp->DestroyComponent();
			RemoveInstanceComponent(OwnedSceneComp);
		}
	}
	GetRootComponent()->SetWorldTransform(PreviousTransform);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	Super::BeginPlay();
	
	if (InteractableInterfaceRef)
	{
		OnActorBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnActorBeginOverlap_Impl);
		OnActorEndOverlap.AddUniqueDynamic(this, &ThisClass::OnActorEndOverlap_Impl);
	}

	SetReplicates( true );

	// 서버에서만 물리 시뮬레이션을 수행
	if ( !HasAuthority() )
	{
		ItemCollision->SetSimulatePhysics( false );
	}
	
	if (GetItemData())
	{
		// 임시: 수량 랜덤
		if (GetItemData()->IsStackableItem())
		{
			int32 RandomNumber = FMath::RandRange(1, GetItemData()->GetItemMaxSlotStackSize());
			GetItemData()->SetQuantity(RandomNumber);
		}
		else
		{
			GetItemData()->SetQuantity(1);
		}
	}
}

void ANAItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UNAItemData* ANAItemActor::GetItemData() const
{
	return UNAItemEngineSubsystem::Get()->GetRuntimeItemData(ItemDataID);
}

bool ANAItemActor::HasValidItemID() const
{
	return !ItemDataID.IsNone();
}

//======================================================================================================================
// Interactable Interface Implements
//======================================================================================================================

bool ANAItemActor::CanInteract_Implementation() const
{
	return IsValid(TriggerSphere)
			&& InteractableInterfaceRef != nullptr && bIsFocused;
}

void ANAItemActor::NotifyInteractableFocusBegin_Implementation(AActor* InteractableActor, AActor* InteractorActor)
{
	if (UNAInteractionComponent* InteractionComp = GetInteractionComponent(InteractorActor))
	{
		bIsFocused = InteractionComp->OnInteractableFound(this);
		if (bIsFocused && ItemWidgetComponent && !ItemWidgetComponent->IsVisible())
		{
			ItemWidgetComponent->ReleaseItemWidgetPopup();
		}
	}
}

void ANAItemActor::NotifyInteractableFocusEnd_Implementation(AActor* InteractableActor, AActor* InteractorActor)
{
	if (UNAInteractionComponent* InteractionComp = GetInteractionComponent(InteractorActor))
	{
		bIsFocused = !InteractionComp->OnInteractableLost(this);
		if (!bIsFocused && !IsPendingKillPending()
			&& ItemWidgetComponent && ItemWidgetComponent->IsVisible())
		{
			ItemWidgetComponent->CollapseItemWidgetPopup();
		}
	}
}

bool ANAItemActor::TryInteract_Implementation(AActor* Interactor)
{
	bIsOnInteract = true;
	SetInteractionPhysicsEnabled(false);
	
	if (Execute_BeginInteract(this, Interactor))
	{
		if (Execute_ExecuteInteract(this, Interactor))
		{
			if (Execute_EndInteract(this, Interactor))
			{
				if (!IsUnlimitedInteractable())
				{
					SetInteractableCount(GetInteractableCount() - 1);
				}
				
				UE_LOG(LogTemp, Warning, TEXT("[TryInteract]  상호작용 사이클 완료"));
				SetInteractionPhysicsEnabled(true);
				bIsOnInteract = false;
				return true;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[TryInteract]  상호작용 사이클 중단"));
	SetInteractionPhysicsEnabled(true);
	bIsOnInteract = false;
	return false;
}

bool ANAItemActor::BeginInteract_Implementation(AActor* InteractorActor)
{
	if (!Execute_CanInteract(this)) { return false; }
	if (!CanPerformInteractionWith(InteractorActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BeginInteract]  상호작용 조건 불충분"));
		return false;
	}
	return bIsOnInteract;
}

bool ANAItemActor::ExecuteInteract_Implementation(AActor* InteractorActor)
{
	ensureAlwaysMsgf(bIsOnInteract, TEXT("[ExecuteInteract_Implementation]  bIsOnInteract이 false였음"));
	return bIsOnInteract;
}

bool ANAItemActor::EndInteract_Implementation(AActor* InteractorActor)
{
	return bIsOnInteract;
}

bool ANAItemActor::IsOnInteract_Implementation() const
{
	return bIsOnInteract;
}

bool ANAItemActor::TryGetInteractableData(FNAInteractableData& OutData) const
{
	if (UNAItemData* ItemData = GetItemData())
	{
		return ItemData->GetInteractableData(OutData);
	}
	return false;
}

bool ANAItemActor::HasInteractionDelay() const
{
	FNAInteractableData Data;
	if (GetItemData() && GetItemData()->GetInteractableData(Data))
	{
		return Data.InteractionDelayTime > 0.f;
	}
	return false;
}

float ANAItemActor::GetInteractionDelay() const
{
	FNAInteractableData Data;
	if (GetItemData() && GetItemData()->GetInteractableData(Data))
	{
		return Data.InteractionDelayTime;
	}
	return 0.f;
}

bool ANAItemActor::IsAttachedAndPendingUse() const
{
	return bIsAttachedAndPendingUse && IsChildActor();
}

void ANAItemActor::SetAttachedAndPendingUse(bool bNewState)
{
	if (bNewState && !IsChildActor())
	{
		ensureAlways(false);
		return;
	}
	
	bIsAttachedAndPendingUse = bNewState;
}

bool ANAItemActor::IsUnlimitedInteractable() const
{
	FNAInteractableData Data;
	if (TryGetInteractableData(Data))
	{
		return Data.bIsUnlimitedInteractable;
	}
	return false;
}

int32 ANAItemActor::GetInteractableCount() const
{
	FNAInteractableData Data;
	if (TryGetInteractableData(Data))
	{
		return Data.InteractableCount;
	}
	return -1;
}

void ANAItemActor::SetInteractableCount(int32 NewCount)
{
	FNAInteractableData Data;
	if (TryGetInteractableData(Data))
	{
		Data.InteractableCount = NewCount;
	}
}

bool ANAItemActor::CanPerformInteractionWith(AActor* Interactor) const
{
	bool bCanPerform = Interactor && GetInteractionComponent(Interactor);
	
	FNAInteractableData Data;
	if (TryGetInteractableData(Data))
	{
		bCanPerform = bCanPerform && Data.InteractableType != ENAInteractableType::None;
		if (Data.bIsUnlimitedInteractable)
		{
			bCanPerform = bCanPerform && Data.InteractableCount > 0;
		}
		return bCanPerform;
	}
	return bCanPerform;
}
