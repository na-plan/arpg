
#include "Item/ItemActor/NAItemActor.h"

#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Interaction/NAInteractionComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "Item/ItemWidget/NAItemWidgetComponent.h"
#include "Net/UnrealNetwork.h"


ANAItemActor::ANAItemActor(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
	    if (!GetClass()->HasAllClassFlags(CLASS_CompiledFromBlueprint))
	    {
	        UE_LOG(LogTemp, Display, TEXT("[ANAItemActor]  C++ CDO 생성자 (%s)"), *GetName());
	    }
	    else
	    {
	        UE_LOG(LogTemp, Display, TEXT("[ANAItemActor]  BP CDO 생성자 (%s)"), *GetName());
	    }
	}
	else
	{
	    if (!GetClass()->HasAllClassFlags(CLASS_CompiledFromBlueprint))
	    {
	        UE_LOG(LogTemp, Display, TEXT("[ANAItemActor]  C++ 인스턴스 생성자 (%s)"), *GetName());
	    }
	    else
	    {
	        UE_LOG(LogTemp, Display, TEXT("[ANAItemActor]  BP 인스턴스 생성자 (%s)"), *GetName());
	    }
	}

	if (UNAItemEngineSubsystem::Get())
	{
		const FNAItemBaseTableRow* MetaData = nullptr;
		if (UNAItemEngineSubsystem::Get()->IsSoftItemMetaDataInitialized())
		{
			if (UNAItemEngineSubsystem::Get()->IsItemMetaDataInitialized())
			{
				MetaData = UNAItemEngineSubsystem::Get()->GetItemMetaDataByClass(GetClass());
			}
			else
			{
				const FDataTableRowHandle* SoftMetaDataHandle = UNAItemEngineSubsystem::Get()->FindSoftItemMetaData(GetClass());
				if (SoftMetaDataHandle && !SoftMetaDataHandle->IsNull())
				{
					MetaData = SoftMetaDataHandle->GetRow<FNAItemBaseTableRow>(SoftMetaDataHandle->RowName.ToString());
				}
			}
		}

		if (MetaData)
		{
			switch (MetaData->CollisionShape)
			{
			case EItemCollisionShape::ICS_Sphere:
				ItemCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ItemCollision(Sphere)"));
				break;
			case EItemCollisionShape::ICS_Box:
				ItemCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ItemCollision(Box)"));
				break;
			case EItemCollisionShape::ICS_Capsule:
				ItemCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("ItemCollision(Capsule)"));
				break;
			default:
				break;
			}
			if (ItemCollision)
			{
				AddInstanceComponent(ItemCollision);
			}
			switch (MetaData->MeshType)
			{
			case EItemMeshType::IMT_Static:
				ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh(Static)"));
				break;
			case EItemMeshType::IMT_Skeletal:
				ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh(Skeletal)"));
				break;
			default:
				break;
			}
			if (ItemMesh)
			{
				AddInstanceComponent(ItemMesh);
			}
		}
	}
	
	TriggerSphere = CreateDefaultSubobject<USphereComponent>("TriggerSphere");
	
	TriggerSphere->SetRelativeLocation(FVector(0.f, 0.f, 140.f));
	TriggerSphere->SetSphereRadius(180.0f);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->CanCharacterStepUpOn = ECB_No;
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); 
	TriggerSphere->SetSimulatePhysics(false);

	ItemWidgetComponent = CreateOptionalDefaultSubobject<UNAItemWidgetComponent>(TEXT("ItemWidgetComponent"));
	
	if (ItemCollision)
	{
		SetRootComponent(ItemCollision);
		if (ItemMesh)
		{
			ItemMesh->SetupAttachment(ItemCollision);
		}
		TriggerSphere->SetupAttachment(ItemCollision);
		ItemWidgetComponent->SetupAttachment(ItemCollision);
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
	
	if (MetaData->CollisionShape != EItemCollisionShape::ICS_None && bNeedItemCollision)
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
	
	if (MetaData->MeshType != EItemMeshType::IMT_None && bNeedItemMesh)
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
#if WITH_EDITOR || WITH_EDITORONLY_DATA
		|| !UNAItemEngineSubsystem::Get()->FindSoftItemMetaData(GetClass())
		|| !UNAItemEngineSubsystem::Get()->IsRegisteredItemMetaClass(GetClass())
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
#if WITH_EDITOR || WITH_EDITORONLY_DATA
	if (!MetaData)
	{
		if (const FDataTableRowHandle* SoftMetaDataHandle = UNAItemEngineSubsystem::Get()->FindSoftItemMetaData(GetClass());
			!SoftMetaDataHandle->IsNull())
		{
			MetaData = SoftMetaDataHandle->GetRow<FNAItemBaseTableRow>(SoftMetaDataHandle->RowName.ToString());
		}
	}
#endif
	if (!MetaData) { return; }

	// 어태치먼트
	FTransform PreviousTransform = GetRootComponent()->GetComponentTransform();

	const EItemSubobjDirtyFlags DirtyFlags = CheckDirtySubobjectFlags(MetaData);
	UClass* NewItemCollisionClass = nullptr;
	UClass* NewItemMeshClass = nullptr;
	if (EnumHasAnyFlags(DirtyFlags, EItemSubobjDirtyFlags::ISDF_CollisionShape))
	{
		switch (MetaData->CollisionShape)
		{
		case EItemCollisionShape::ICS_Sphere:
			NewItemCollisionClass = USphereComponent::StaticClass();
			break;
		case EItemCollisionShape::ICS_Box:
			NewItemCollisionClass = UBoxComponent::StaticClass();
			break;
		case EItemCollisionShape::ICS_Capsule:
			NewItemCollisionClass = UCapsuleComponent::StaticClass();
			break;
		default:
			break;
		}
		if (NewItemCollisionClass && ItemCollision && ItemCollision->GetClass() != NewItemCollisionClass)
		{
			ItemCollision->ClearFlags(RF_Standalone | RF_Public);
			ItemCollision->DestroyComponent();
			RemoveInstanceComponent(ItemCollision);
		}
	}
	if (EnumHasAnyFlags(DirtyFlags, EItemSubobjDirtyFlags::ISDF_MeshType))
	{
		switch (MetaData->MeshType)
		{
		case EItemMeshType::IMT_Skeletal:
			NewItemMeshClass = USkeletalMeshComponent::StaticClass();
			break;
		case EItemMeshType::IMT_Static:
			NewItemMeshClass = UStaticMeshComponent::StaticClass();
			break;
		default:
			break;
		}
		if (NewItemMeshClass && ItemMesh && ItemMesh->GetClass() != NewItemMeshClass)
		{
			ItemMesh->ClearFlags(RF_Standalone | RF_Public);
			ItemMesh->DestroyComponent();
			RemoveInstanceComponent(ItemMesh);
		}
	}
	for (UActorComponent* OwnedComponent : GetComponents().Array())
	{
		if (!OwnedComponent) continue;

		if (NewItemCollisionClass && OwnedComponent->GetClass()->IsChildOf(NewItemCollisionClass))
		{
			if (UShapeComponent* NewItemCollision = Cast<UShapeComponent>(OwnedComponent))
			{
				ItemCollision = NewItemCollision;
				AddInstanceComponent(ItemCollision);
			}
		}
		if (NewItemMeshClass && OwnedComponent->GetClass()->IsChildOf(NewItemMeshClass))
		{
			if (UMeshComponent* NewItemMesh = Cast<UMeshComponent>(OwnedComponent))
			{
				ItemMesh = NewItemMesh;
				AddInstanceComponent(ItemMesh);
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

	if (ItemCollision)
	{
		SetRootComponent(ItemCollision);
		AddReplicatedSubObject(ItemCollision);
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
				
				if (OwnedSceneComp != GetRootComponent())
				{
					OwnedSceneComp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
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

    if (GetRootComponent() && GetRootComponent() == ItemCollision)
    {
	    GetRootComponent()->SetWorldTransform(PreviousTransform);
    }
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
