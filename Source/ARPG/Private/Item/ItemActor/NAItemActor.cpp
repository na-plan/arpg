
#include "Item/ItemActor/NAItemActor.h"

#include "LandscapeGizmoActiveActor.h"
#include "MovieSceneTracksComponentTypes.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/PackageMapClient.h"
#include "Interaction/NAInteractionComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "Item/ItemWidget/NAItemWidgetComponent.h"
#include "Net/UnrealNetwork.h"


ANAItemActor::ANAItemActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StubRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("FakeRootComponent"));
	SetRootComponent( StubRootComponent );
	ItemMesh = CreateOptionalDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh(Static)"));
	TriggerSphere = CreateDefaultSubobject<USphereComponent>("TriggerSphere");
	ItemWidgetComponent = CreateOptionalDefaultSubobject<UNAItemWidgetComponent>(TEXT("ItemWidgetComponent"));
	
	if ( ItemMesh )
	{
		ItemMesh->SetupAttachment( StubRootComponent );
		ItemMesh->SetSimulatePhysics( false );
		ItemMesh->SetCollisionEnabled( ECollisionEnabled::NoCollision );
	}
	
	TriggerSphere->SetRelativeLocation(FVector(0.f, 0.f, 140.f));
	TriggerSphere->SetSphereRadius(180.0f);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->CanCharacterStepUpOn = ECB_No;
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); 
	TriggerSphere->SetSimulatePhysics(false);
	TriggerSphere->SetupAttachment( StubRootComponent );

	if ( ItemWidgetComponent )
	{
		ItemWidgetComponent->SetupAttachment( StubRootComponent );	
	}

	bReplicateUsingRegisteredSubObjectList = true;
	ItemDataID = NAME_None;
	bReplicates = true;
	AActor::SetReplicateMovement( true );
	bAlwaysRelevant = true;
}

void ANAItemActor::PostInitProperties()
{
	Super::PostInitProperties();

	// ensureAlways(ItemCollision && GetRootComponent() == ItemCollision);
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
	InitCheckIfChildActor();
}

bool ANAItemActor::UpgradeLegacyBlueprints()
{
	if ( HasAnyFlags( RF_ClassDefaultObject ) )
	{
		return false;
	}

	bool bOldVersion = false;
	if ( Version == 0 )
	{
		if ( StubRootComponent != nullptr && GetRootComponent() != StubRootComponent )
		{
			if ( USceneComponent* LegacyComponent = GetRootComponent();
				 LegacyComponent == ItemCollision ) 
			{
				ItemCollision = nullptr;
			}

			TArray<USceneComponent*> LegacyChildren;
			const TSet<UActorComponent*> ActorChildrenComponent = GetComponents();

			for ( auto It = ActorChildrenComponent.CreateConstIterator(); It; ++It )
			{
				if ( (*It)->GetName().StartsWith( "ItemCollision" ) &&
					 (*It)->GetClass()->IsChildOf( UShapeComponent::StaticClass() ) )
				{
					const USceneComponent* SceneCast = Cast<USceneComponent>( *It );
					for ( const auto CIt = SceneCast->GetAttachChildren().CreateConstIterator(); CIt; )
					{
						LegacyChildren.Add( *CIt );
						(*CIt)->DetachFromComponent( FDetachmentTransformRules::KeepRelativeTransform );
					
					}
					(*It)->ClearFlags(RF_Standalone | RF_Public);
					(*It)->DestroyComponent();
					RemoveInstanceComponent( *It );
					RemoveOwnedComponent( *It );
				}
			}

			if ( LegacyChildren.Num() )
			{
				for ( USceneComponent* Orphan : LegacyChildren )
				{
					Orphan->AttachToComponent( StubRootComponent, FAttachmentTransformRules::KeepRelativeTransform );
				}
			}

			SetRootComponent( StubRootComponent );

			// 에셋 업데이트
			Version = 1;
			bOldVersion = true;
			
			GetPackage()->MarkPackageDirty();
		}
	}

	return bOldVersion;
}

EItemSubobjDirtyFlags ANAItemActor::GetDirtySubobjectFlags(const FNAItemBaseTableRow* MetaData) const
{
	EItemSubobjDirtyFlags DirtyFlags = EItemSubobjDirtyFlags::ISDF_None;
	if ( !MetaData )
	{
		ensure(false);
		return DirtyFlags;
	}
	
	if (MetaData->CollisionShape != EItemCollisionShape::ICS_None && bNeedItemCollision)
	{
		bool bDirtyShape = false;
		bDirtyShape |= ItemCollision == nullptr;
		if ( ItemCollision )
		{
			const FCollisionShape Shape = ItemCollision->GetCollisionShape();
			switch (MetaData->CollisionShape)
			{
			case EItemCollisionShape::ICS_Sphere:
				bDirtyShape |= !Shape.IsSphere();
				bDirtyShape |= Shape.GetSphereRadius() != MetaData->CollisionSphereRadius;
				break;
			case EItemCollisionShape::ICS_Box:
				bDirtyShape |= !Shape.IsBox();
				bDirtyShape |= Shape.GetExtent() != MetaData->CollisionBoxExtent;
				break;
			case EItemCollisionShape::ICS_Capsule:
				bDirtyShape |= !Shape.IsCapsule();
				bDirtyShape |= Shape.GetCapsuleRadius() != MetaData->CollisionCapsuleSize.X;
				bDirtyShape |= Shape.GetCapsuleHalfHeight() != MetaData->CollisionCapsuleSize.Y;
				break;
			default:
				ensure(false);
				break;
			}
		}

		if ( bDirtyShape )
		{
			EnumAddFlags( DirtyFlags, EItemSubobjDirtyFlags::ISDF_CollisionShape );
		}
	}
	
	if (MetaData->MeshType != EItemMeshType::IMT_None && bNeedItemMesh)
	{
		bool bDirtyMesh = false;

		bDirtyMesh |= ItemMesh == nullptr;
		if ( ItemMesh )
		{
			switch (MetaData->MeshType)
			{
			case EItemMeshType::IMT_Static:
				{
					UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>( ItemMesh );
					bDirtyMesh |= StaticMeshComponent == nullptr;
					if ( StaticMeshComponent )
					{
						bDirtyMesh |= StaticMeshComponent->GetStaticMesh() != MetaData->StaticMeshAssetData.StaticMesh;
						bDirtyMesh |= ItemFractureCollection != MetaData->StaticMeshAssetData.FractureCollection;
						bDirtyMesh |= ItemFractureCache != MetaData->StaticMeshAssetData.FractureCache;
					}
					break;
				}
			case EItemMeshType::IMT_Skeletal:
				{
					USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>( ItemMesh );
					bDirtyMesh |= SkeletalMeshComponent == nullptr;
					if ( SkeletalMeshComponent )
					{
						bDirtyMesh |= SkeletalMeshComponent->GetSkeletalMeshAsset() != MetaData->SkeletalMeshAssetData.SkeletalMesh;
						bDirtyMesh |= SkeletalMeshComponent->GetAnimClass() != MetaData->SkeletalMeshAssetData.AnimClass;
					}
					break;
				}
			default:
				ensure(false);
				break;
			}
		}
		
		if ( bDirtyMesh )
		{
			EnumAddFlags( DirtyFlags, EItemSubobjDirtyFlags::ISDF_MeshType );
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
	
	UNAItemEngineSubsystem* Subsystem = UNAItemEngineSubsystem::Get();
	
	if (!Subsystem || !Subsystem->IsItemMetaDataInitialized()
#if WITH_EDITOR
		|| !Subsystem->IsRegisteredItemMetaClass(GetClass())
#endif
		)
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

    // 구버전 블루프린트인 경우 Stub 루트 컴포넌트로 다시 구성
#if WITH_EDITOR
	UpgradeLegacyBlueprints();
#endif
	const EItemSubobjDirtyFlags DirtyFlags = GetDirtySubobjectFlags( MetaData );

	// 루트 컴포넌트 기준으로 재구성
	USceneComponent* CandidateRootComponent = StubRootComponent;

	// 기존 루트의 월드 트랜스폼을 저장
	const FTransform PreviousTransform = RootComponent->GetComponentTransform();
	EObjectFlags SubobjFlags = GetMaskedFlags(RF_PropagateToSubObjects);

	// Sanity Check: 콜리전 설정 또는 콜리전 생성 요청이 conflict하는 경우
	check( !((!bNeedItemCollision) ^ (MetaData->CollisionShape == EItemCollisionShape::ICS_None)) );
	
	// 아이템 콜리전은 처음에 만들어지지 않기 때문에 한번 체크
	if ( bNeedItemCollision &&
		 MetaData->CollisionShape != EItemCollisionShape::ICS_None &&
		 ( EnumHasAnyFlags( DirtyFlags, EItemSubobjDirtyFlags::ISDF_CollisionShape ) || !ItemCollision ) )
	{
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
			break;
		}
		ensure(ItemCollision != nullptr);

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
	}

	// 아이템 매시가 ObjectInitializer에서 Optional로 생성되지 않을 수 있음
	if ( bNeedItemMesh &&
		 MetaData->MeshType != EItemMeshType::IMT_None &&
		 ( EnumHasAnyFlags( DirtyFlags, EItemSubobjDirtyFlags::ISDF_MeshType ) || !ItemMesh ))
	{
		if ( ItemMesh )
		{
			switch (MetaData->MeshType)
			{
			case EItemMeshType::IMT_Static:
				// 기존의 컴포넌트 재사용 (기본 컴포넌트가 스태틱 매쉬임)
				break;
			case EItemMeshType::IMT_Skeletal:
				ItemMesh = NewObject<USkeletalMeshComponent>(this, TEXT("ItemMesh(Skeletal)"), SubobjFlags);
				break;
			default:
				ensure(false);
				break;
			}
		}
		ensure(ItemMesh != nullptr);
		
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
	}

	if ( ItemCollision )
	{
		CandidateRootComponent = ItemCollision;
	}
	
	// 어태치먼트
    if ( GetRootComponent() != CandidateRootComponent )
	{
    	TArray<USceneComponent*> ChildrenComponents;

		if ( const USceneComponent* OldRoot = GetRootComponent() )
		{
			ChildrenComponents = OldRoot->GetAttachChildren();
			for ( auto It = ChildrenComponents.CreateConstIterator(); It; ++It )
			{
				(*It)->DetachFromComponent( FDetachmentTransformRules::KeepRelativeTransform );
			}

			// 만약 가짜 루트 컴포넌트였다면 프로퍼티에서 참조되지 않도록 지움
			if ( RootComponent == StubRootComponent && CandidateRootComponent != StubRootComponent )
			{
				StubRootComponent = nullptr;
			}
		}
		
		SetRootComponent( CandidateRootComponent );
		
		if ( ChildrenComponents.Num() )
		{
			for ( USceneComponent* Child : ChildrenComponents )
			{
				Child->AttachToComponent( ItemCollision, FAttachmentTransformRules::KeepRelativeTransform );
			}
		}
	}

	// 부모, 자식에서 Property로 설정된 컴포넌트들을 조회
	// 최종적으로 프로퍼티에 남은 컴포넌트 주소들을 확인
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

	// 마지막으로 등록된 컴포넌트들을 순회하면서 프로퍼티에 없는 컴포넌트들을 삭제
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

	// BeginPlay 시점에서 물리 설정
	if ( ItemCollision )
	{
		ItemCollision->SetSimulatePhysics( false );
		ItemCollision->SetCollisionEnabled( ECollisionEnabled::NoCollision );
		ItemCollision->SetIsReplicated( true );
		ItemCollision->SetGenerateOverlapEvents( true );
		ItemCollision->SetNetAddressable();
		if ( GetWorld()->IsGameWorld() )
		{
			AddReplicatedSubObject( ItemCollision );	
		}
	}
	// 아이템 콜리전이 아이템의 물리 처리를 주도
	if ( ItemCollision && ItemMesh )
	{
		ItemMesh->SetSimulatePhysics( false );
		ItemMesh->SetCollisionEnabled( ECollisionEnabled::NoCollision );
		ItemMesh->SetGenerateOverlapEvents( false );
	}
	
	GetRootComponent()->SetWorldTransform(PreviousTransform);
}

void ANAItemActor::Destroyed()
{
	Super::Destroyed();

	if (!HasActorBegunPlay()) return;
	CollapseItemWidgetComponent();
}

void ANAItemActor::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
	DOREPLIFETIME( ANAItemActor, bWasChildActor );
	DOREPLIFETIME( ANAItemActor, ItemCollision );
}

bool ANAItemActor::ReplicateSubobjects( UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags )
{
	bool WroteSomething = Super::ReplicateSubobjects( Channel, Bunch, RepFlags );

	if ( ItemCollision )
	{
		WroteSomething |= ItemCollision->ReplicateSubobjects( Channel, Bunch, RepFlags );
	}

	return WroteSomething;
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

void ANAItemActor::ReleaseItemWidgetComponent()
{
	if (ItemWidgetComponent && !ItemWidgetComponent->IsVisible())
	{
		ItemWidgetComponent->ReleaseItemWidgetPopup();
	}
}

void ANAItemActor::CollapseItemWidgetComponent()
{
	if (ItemWidgetComponent && ItemWidgetComponent->IsVisible())
	{
		ItemWidgetComponent->CollapseItemWidgetPopup();
	}
}

void ANAItemActor::OnFullyAddedToInventoryBeforeDestroy(AActor* Interactor)
{
	OnFullyAddedToInventoryBeforeDestroy_Impl(Interactor);
	Destroy();
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

void ANAItemActor::InitCheckIfChildActor()
{
	if ( HasAuthority() )
	{
		bWasChildActor = IsChildActor();
	}
	
	// ChildActorComponent에 의해 생성된 경우
	if ( bWasChildActor || GetAttachParentActor() || ( RootComponent && RootComponent->GetAttachParent() && RootComponent->GetAttachParent()->IsA<UChildActorComponent>() ) )
	{
		if (ItemCollision)
		{
			ItemCollision->SetSimulatePhysics(false);
			ItemCollision->SetGenerateOverlapEvents( false );
			ItemCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (TriggerSphere)
		{
			TriggerSphere->Deactivate();
			TriggerSphere->SetSimulatePhysics(false);
			TriggerSphere->SetGenerateOverlapEvents(false);
			TriggerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (ItemWidgetComponent)
		{
			CollapseItemWidgetComponent();
			ItemWidgetComponent->Deactivate();
			ItemWidgetComponent->SetVisibility(false);
		}
		if (ItemMesh)
		{
			ItemMesh->SetCollisionEnabled( ECollisionEnabled::NoCollision );
			ItemMesh->SetSimulatePhysics( false );
			ItemMesh->SetGenerateOverlapEvents(false);
		}
	}
	else
	{
		// 트랜스폼 및 콜리전, 피직스 등등 설정 여기에
		if (ItemCollision)
		{
			ItemCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			ItemCollision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
			if ( HasAuthority() )
			{
				// 서버에서만 물리 시뮬레이션을 수행
				ItemCollision->SetSimulatePhysics( true );	
			}
			else
			{
				ItemCollision->SetSimulatePhysics( false );
			}
			ItemCollision->SetGenerateOverlapEvents( true );
			ItemCollision->SetIsReplicated( true );
		}
		if ( ItemMesh )
		{
			ItemMesh->SetCollisionEnabled( ECollisionEnabled::NoCollision );
			ItemMesh->SetSimulatePhysics( false );
			ItemMesh->SetGenerateOverlapEvents( false );
		}
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

void ANAItemActor::OnRep_ItemCollision( UShapeComponent* PreviousComponent )
{
	if ( ItemCollision )
	{
		TArray<USceneComponent*> PreviousChildren;

		if ( PreviousComponent )
		{
			const TArray<USceneComponent*>& ChildrenArray = PreviousComponent->GetAttachChildren();
			for ( auto It = ChildrenArray.CreateConstIterator(); It; ++It  )
			{
				PreviousChildren.Add(*It);
				(*It)->DetachFromComponent( FDetachmentTransformRules::KeepRelativeTransform );
			}
		}
		
		if ( !ItemCollision->IsRegistered() )
		{
			ItemCollision->RegisterComponent();

			for ( USceneComponent* Child : PreviousChildren )
			{
				Child->AttachToComponent( ItemCollision, FAttachmentTransformRules::KeepRelativeTransform );
			}
		}
	}
}

void ANAItemActor::BeginPlay()
{
	Super::BeginPlay();
	InitCheckIfChildActor();
	
	if (InteractableInterfaceRef)
	{
		OnActorBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnActorBeginOverlap_Impl);
		OnActorEndOverlap.AddUniqueDynamic(this, &ThisClass::OnActorEndOverlap_Impl);
	}
	
	if (HasValidItemID())
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

bool ANAItemActor::IsOnInteract_Implementation() const
{
	return bIsOnInteract;
}

void ANAItemActor::NotifyInteractableFocusBegin_Implementation(AActor* InteractableActor, AActor* InteractorActor)
{
	if (UNAInteractionComponent* InteractionComp = GetInteractionComponent(InteractorActor))
	{
		bIsFocused = InteractionComp->OnInteractableFound(this);
		if (bIsFocused)
		{
			ReleaseItemWidgetComponent();
		}
	}
}

void ANAItemActor::NotifyInteractableFocusEnd_Implementation(AActor* InteractableActor, AActor* InteractorActor)
{
	if (UNAInteractionComponent* InteractionComp = GetInteractionComponent(InteractorActor))
	{
		bIsFocused = !InteractionComp->OnInteractableLost(this);
		if (!bIsFocused && !IsPendingKillPending())
		{
			CollapseItemWidgetComponent();
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
		if (!Data.bIsUnlimitedInteractable)
		{
			bCanPerform = bCanPerform && Data.InteractableCount > 0;
		}
		return bCanPerform;
	}
	return bCanPerform;
}
