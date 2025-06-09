
#include "Item/ItemActor/NAItemActor.h"

#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Interaction/NAInteractionComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/TextRenderComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"


ANAItemActor::ANAItemActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ItemRootShape = CreateDefaultSubobject<USphereComponent>(TEXT("ItemRootShape(Sphere)"));
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh(Static)"));
	
	ItemInteractionButton = CreateDefaultSubobject<UBillboardComponent>(TEXT("ItemInteractionButton"));
	ItemInteractionButton->SetupAttachment(ItemMesh);
	
	ItemInteractionButtonText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ItemInteractionButtonText"));
	ItemInteractionButtonText->SetupAttachment(ItemInteractionButton);
	
	ItemDataID = NAME_None;

	bReplicates = true;
	AActor::SetReplicateMovement( true );
	bAlwaysRelevant = true;
}

#if WITH_EDITOR
void ANAItemActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

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
	
	if (ItemDataID.IsNone() && !GetWorld()->IsPreviewWorld())
	{
		InitItemData();
	}

	const FNAItemBaseTableRow* MetaData = UNAItemEngineSubsystem::Get()->GetItemMetaDataByClass(GetClass());
	if (!MetaData) { return; }
	
	EItemSubobjDirtyFlags DirtyFlags = CheckDirtySubobjectFlags(MetaData);
	FTransform CachedRootWorldTransform = GetRootComponent()->GetComponentTransform();
	if (DirtyFlags != EItemSubobjDirtyFlags::MF_None)
	{
		EObjectFlags SubobjFlags = GetMaskedFlags(RF_PropagateToSubObjects);
		
		TArray<UActorComponent*> OldComponents;
		OldComponents.Reset();
		
		if (EnumHasAnyFlags(DirtyFlags, EItemSubobjDirtyFlags::MF_RootShape))
		{
			if (IsValid(ItemRootShape))
			{
				OldComponents.Emplace(ItemRootShape);
			}
			
			switch (MetaData->RootShapeType)
			{
			case EItemRootShapeType::IRT_Sphere:
				ItemRootShape = NewObject<USphereComponent>(this, TEXT("ItemRootShape(Sphere)"), SubobjFlags);
				break;
			case EItemRootShapeType::IRT_Box:
				ItemRootShape = NewObject<UBoxComponent>(this, TEXT("ItemRootShape(Box)"), SubobjFlags);
				break;
			case EItemRootShapeType::IRT_Capsule:
				ItemRootShape = NewObject<UCapsuleComponent>(this, TEXT("ItemRootShape(Capsule)"), SubobjFlags);
				break;
			default:
				ensure(false);
				break;
			}
			ensure(ItemRootShape != nullptr);
		}
		
		if (EnumHasAnyFlags(DirtyFlags, EItemSubobjDirtyFlags::MF_Mesh))
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
		
		if (EnumHasAnyFlags(DirtyFlags, EItemSubobjDirtyFlags::MF_IxButtonSprite))
		{
			ItemInteractionButton->SetSprite(MetaData->IconAssetData.IxButtonIcon);
		}

		if (EnumHasAnyFlags(DirtyFlags, EItemSubobjDirtyFlags::MF_IxButtonText))
		{
			FNAInteractableData InteractableData;
			if (GetInteractableData_Internal(InteractableData))
			{
				ItemInteractionButtonText->SetText(InteractableData.InteractionName);
			}
		}
		
		if (!OldComponents.IsEmpty())
		{
			for (UActorComponent* OldComponent : OldComponents)
			{
				OldComponent->ClearFlags(RF_Standalone | RF_Public);
				OldComponent->DestroyComponent();
				
				// Actor의 Components 배열에서도 제거
				if (AActor* MyOwner = OldComponent->GetOwner())
				{
					MyOwner->RemoveInstanceComponent(OldComponent);
				}
			}
			OldComponents.Empty();
		}
	}
	
	SetRootComponent(ItemRootShape);
	ItemMesh->AttachToComponent(ItemRootShape, FAttachmentTransformRules::KeepRelativeTransform);
	ItemInteractionButton->AttachToComponent(ItemMesh, FAttachmentTransformRules::KeepRelativeTransform);
	ItemInteractionButtonText->AttachToComponent(ItemInteractionButton, FAttachmentTransformRules::KeepRelativeTransform);

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
				OwnedSceneComp->RegisterComponent();
				AddInstanceComponent(OwnedSceneComp);
				continue;
			}

			// AttachChildren 복사본 사용 (Detach 중 배열 변화 방지)
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

			// Actor의 Components 배열에서도 제거
			if (AActor* MyOwner = OwnedSceneComp->GetOwner())
			{
				MyOwner->RemoveInstanceComponent(OwnedSceneComp);
			}
		}
	}
	
	if (USphereComponent* RootSphere = Cast<USphereComponent>(ItemRootShape))
	{
		RootSphere->SetSphereRadius(MetaData->RootSphereRadius);
	}
	else if (UBoxComponent* RootBox = Cast<UBoxComponent>(ItemRootShape))
	{
		RootBox->SetBoxExtent(MetaData->RootBoxExtent);
	}
	else if (UCapsuleComponent* RootCapsule = Cast<UCapsuleComponent>(ItemRootShape))
	{
		RootCapsule->SetCapsuleSize(MetaData->RootCapsuleSize.X, MetaData->RootCapsuleSize.Y);
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

	// 트랜스폼 및 콜리전, 피직스 등등 설정 여기에
	ItemRootShape->SetWorldTransform(CachedRootWorldTransform);
	ItemRootShape->SetWorldScale3D(ItemRootShape->GetComponentScale() * MetaData->RootShapeScaleFactor);
	ItemMesh->SetRelativeTransform(MetaData->MeshTransform);
	ItemInteractionButton->SetRelativeTransform(MetaData->IxButtonTransform);
	ItemInteractionButtonText->SetRelativeTransform(MetaData->IxButtonTextTransform);
}

void ANAItemActor::PostLoad()
{
	Super::PostLoad();

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		if (ItemDataID.IsNone() && !GetWorld()->IsPreviewWorld())
		{
			InitItemData();
		}
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

void ANAItemActor::OnItemDataInitialized_Implementation()
{
	VerifyInteractableData();
}

EItemSubobjDirtyFlags ANAItemActor::CheckDirtySubobjectFlags(const FNAItemBaseTableRow* MetaData) const
{
	EItemSubobjDirtyFlags DirtyFlags = EItemSubobjDirtyFlags::MF_None;
	
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
	
	if (!IsValid(ItemRootShape))
	{
		EnumAddFlags(DirtyFlags, EItemSubobjDirtyFlags::MF_RootShape);
	}
	else
	{
		switch (MetaData->RootShapeType)
		{
		case EItemRootShapeType::IRT_Sphere:
			if (!ItemRootShape->IsA<USphereComponent>())
			{
				EnumAddFlags(DirtyFlags, EItemSubobjDirtyFlags::MF_RootShape);
			}
			break;
		
		case EItemRootShapeType::IRT_Box:
			if (!ItemRootShape->IsA<UBoxComponent>())
			{
				EnumAddFlags(DirtyFlags, EItemSubobjDirtyFlags::MF_RootShape);
			}
			break;
		
		case EItemRootShapeType::IRT_Capsule:
			if (!ItemRootShape->IsA<UCapsuleComponent>())
			{
				EnumAddFlags(DirtyFlags, EItemSubobjDirtyFlags::MF_RootShape);
			}
			break;
		
		default:
			ensure(false);
			break;
		}
	}
	
	if (!IsValid(ItemMesh))
	{
		EnumAddFlags(DirtyFlags,
			EItemSubobjDirtyFlags::MF_Mesh);
	}
	else
	{
		switch (MetaData->MeshType)
		{
		case EItemMeshType::IMT_Static:
			if (!ItemMesh->IsA<UStaticMeshComponent>())
			{
				EnumAddFlags(DirtyFlags,
					EItemSubobjDirtyFlags::MF_Mesh);
			}
			break;
		
		case EItemMeshType::IMT_Skeletal:
			if (!ItemMesh->IsA<USkeletalMeshComponent>())
			{
				EnumAddFlags(DirtyFlags,
					EItemSubobjDirtyFlags::MF_Mesh);
			}
			break;
		
		default:
			ensure(false);
			break;
		}
	}

	if (!ensure(IsValid(ItemInteractionButton)))
	{
		return DirtyFlags;
	}
	if (MetaData->IconAssetData.IxButtonIcon != ItemInteractionButton->Sprite)
	{
		EnumAddFlags(DirtyFlags, EItemSubobjDirtyFlags::MF_IxButtonSprite);
	}

	if (!ensure(IsValid(ItemInteractionButtonText)))
	{
		return DirtyFlags;
	}
	FNAInteractableData InteractableData;
	if (GetInteractableData_Internal(InteractableData))
	{
		if (InteractableData.InteractionName.ToString() != ItemInteractionButtonText->Text.ToString())
		{
			EnumAddFlags(DirtyFlags, EItemSubobjDirtyFlags::MF_IxButtonText);
		}
	}
	
	return DirtyFlags;
}

void ANAItemActor::OnActorBeginOverlap_Impl(AActor* OverlappedActor, AActor* OtherActor)
{
	Execute_NotifyInteractableFocusBegin(this, OverlappedActor, OtherActor);
}

void ANAItemActor::OnActorEndOverlap_Impl(AActor* OverlappedActor, AActor* OtherActor)
{
	Execute_NotifyInteractableFocusEnd(this, OverlappedActor, OtherActor);
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
		ensure(false);
	}
}

void ANAItemActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (Execute_CanInteract(this))
	{
		OnActorBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnActorBeginOverlap_Impl);
		OnActorEndOverlap.AddUniqueDynamic(this, &ThisClass::OnActorEndOverlap_Impl);
	}
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

FNAInteractableData ANAItemActor::GetInteractableData_Implementation() const
{
	FNAInteractableData InteractableData;
	GetInteractableData_Internal(InteractableData);
	return InteractableData;
}

bool ANAItemActor::GetInteractableData_Internal(FNAInteractableData& OutIxData) const
{
	const FNAInteractableData* InteractableDataRef = nullptr;
	const UNAItemData* RuntimeItemData = UNAItemEngineSubsystem::Get()->GetRuntimeItemData(ItemDataID);
	if (RuntimeItemData)
	{
		if (const FNAItemBaseTableRow* ItemMetaData = RuntimeItemData->GetItemMetaDataStruct<FNAItemBaseTableRow>())
		{
			InteractableDataRef = TryGetInteractableData(ItemMetaData);
		}
	}
	if (InteractableDataRef)
	{
		OutIxData = *InteractableDataRef;
	}
	return InteractableDataRef ? true : false;
}

bool ANAItemActor::CanInteract_Implementation() const
{
	return IsValid(ItemRootShape) && InteractableInterfaceRef != nullptr;
}

// bool ANAItemActor::CanInteract_Implementation() const
// {
// 	return Execute_CanInteract(this)/* && Execute_GetInteractableData(this).InteractingCharacter.IsValid()*/;
// }

void ANAItemActor::NotifyInteractableFocusBegin_Implementation(AActor* InteractableActor, AActor* InteractorActor)
{
	bIsFocused = true;
	
	if (Execute_CanInteract(this))
	{
		if (UNAInteractionComponent* InteractionComp = TryGetInteractionComponent(InteractorActor))
		{
			const bool bSucceed = InteractionComp->OnInteractableFound(this);
			// @TODO: ANAItemActor 쪽에서 '상호작용 버튼 위젯' release?
		}
	}
}

void ANAItemActor::NotifyInteractableFocusEnd_Implementation(AActor* InteractableActor, AActor* InteractorActor)
{
	bIsFocused = false;
	
	if (Execute_CanInteract(this))
	{
		if (UNAInteractionComponent* InteractionComp = TryGetInteractionComponent(InteractorActor))
		{
			const bool bSucceed = InteractionComp->OnInteractableLost(this);
			// @TODO: ANAItemActor 쪽에서 '상호작용 버튼 위젯' collapse?
		}
	}
}

void ANAItemActor::BeginInteract_Implementation(AActor* InteractorActor)
{
	//if (UNAInteractionComponent* InteractionComp = TryGetInteractionComponent(InteractorActor))
	//{
	bIsOnInteract = true;

	// @TODO: 상호작용 시작 시 필요한 로직이 있으면 여기에 추가, 상호작용 시작을 알리는 이벤트라고 생각하면 됨
	//}
}

void ANAItemActor::EndInteract_Implementation(AActor* InteractorActor)
{
	if (UNAInteractionComponent* InteractionComp = TryGetInteractionComponent(InteractorActor))
	{
		bIsOnInteract = false;
		InteractionComp->OnInteractionEnded(InteractableInterfaceRef);
		// @TODO: 상호작용 종료 시 필요한 로직이 있으면 여기에 추가, 상호작용 종료를 알리는 이벤트라고 생각하면 됨
	}
}

bool ANAItemActor::ExecuteInteract_Implementation(AActor* InteractorActor)
{
	ensureAlwaysMsgf(bIsOnInteract, TEXT("[INAInteractableInterface::ExecuteInteract_Implementation]  bIsOnInteract이 false였음"));
	
	//if (UNAInteractionComponent* InteractionComp = TryGetInteractionComponent(InteractorActor))
	//{

	// @TODO: 상호작용 실행에 필요한 로직이 있으면 여기에 추가
	//}
	return false;
}

bool ANAItemActor::IsOnInteract_Implementation() const
{
	return bIsOnInteract;
}