
#include "Item/ItemActor/NAItemActor.h"

#include "Item/EngineSubsystem/NAItemEngineSubsystem.h"
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
	SetRootComponent(ItemRootShape);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh(Static)"));
	ItemMesh->SetupAttachment(ItemRootShape);
	
	ItemInteractionButton = CreateDefaultSubobject<UBillboardComponent>(TEXT("ItemInteractionButton"));
	ItemInteractionButton->SetupAttachment(ItemMesh);
	
	ItemInteractionButtonText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ItemInteractionButtonText"));
	ItemInteractionButtonText->SetupAttachment(ItemInteractionButton);
	
	ItemDataID = NAME_None;
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

	if (!UNAItemEngineSubsystem::Get() || !UNAItemEngineSubsystem::Get()->IsItemMetaDataInitialized())
	{
		return;
	}

	if (!HasAnyFlags(RF_ClassDefaultObject) && !GetWorld()->IsPreviewWorld())
	{
		if (ItemDataID.IsNone())
		{
			InitItemData();
		}
	}
	const FNAItemBaseTableRow* MetaData = UNAItemEngineSubsystem::Get()->GetItemMetaDataByClass(GetClass());
	if (!MetaData) { return; }
	
	EItemSubobjDirtyFlags DirtyFlags = CheckDirtySubobjectFlags(MetaData);
	FTransform CachedRootWorldTransform = GetRootComponent()->GetComponentTransform();
	TArray<USceneComponent*> NewComponents;
	NewComponents.Reset();
	if (DirtyFlags != EItemSubobjDirtyFlags::MF_None)
	{
		EObjectFlags SubobjFlags = GetMaskedFlags(RF_PropagateToSubObjects) | RF_DefaultSubObject;
		
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
				if (USphereComponent* RootSphere = GetComponentByClass<USphereComponent>())
				{
					if (IsValid(RootSphere))
					{
						ItemRootShape = RootSphere;
					}
					break;
				}
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
			if (ensure(ItemRootShape != nullptr))
			{
				NewComponents.Add(ItemRootShape);
			}
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
				if (UStaticMeshComponent* StaticMeshComp = GetComponentByClass<UStaticMeshComponent>())
				{
					if (IsValid(StaticMeshComp))
					{
						ItemMesh =  StaticMeshComp;
					}
					break;
				}
				ItemMesh = NewObject<UStaticMeshComponent>(this, TEXT("ItemMesh(Static)"), SubobjFlags);
				break;
			case EItemMeshType::IMT_Skeletal:
				ItemMesh = NewObject<USkeletalMeshComponent>(this, TEXT("ItemMesh(Skeletal)"), SubobjFlags);
				break;
			default:
				ensure(false);
				break;
			}

			if (ensure(ItemMesh != nullptr))
			{
				NewComponents.Add(ItemMesh);
			}
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
				OldComponent->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors);
				OldComponent->ClearFlags(RF_Standalone | RF_Public);
				OldComponent->DestroyComponent();
				
				// Actor의 Components 배열에서도 제거
				if (AActor* MyOwner = OldComponent->GetOwner())
				{
					MyOwner->RemoveOwnedComponent(OldComponent);
					MyOwner->RemoveInstanceComponent(OldComponent);
				}
			}
			OldComponents.Empty();
		}
			
		if (!NewComponents.IsEmpty())
		{
			for (USceneComponent* NewComponent : NewComponents)
			{
				NewComponent->RegisterComponent();
				AddInstanceComponent(NewComponent);
			}
			NewComponents.Empty();
		}
	}

	for (UActorComponent* OwnedComponent : GetComponents().Array())
	{
		if (USceneComponent* OwnedSceneComp = Cast<USceneComponent>(OwnedComponent))
		{
			if (OwnedSceneComp == ItemRootShape
				|| OwnedSceneComp == ItemMesh
				|| OwnedSceneComp == ItemInteractionButton
				|| OwnedSceneComp == ItemInteractionButtonText)
			{
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

			OwnedSceneComp->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors);
			OwnedSceneComp->ClearFlags(RF_Standalone | RF_Public);
			OwnedSceneComp->DestroyComponent();

			// Actor의 Components 배열에서도 제거
			if (AActor* MyOwner = OwnedSceneComp->GetOwner())
			{
				MyOwner->RemoveOwnedComponent(OwnedSceneComp);
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
	
	SetRootComponent(ItemRootShape);
	ItemMesh->AttachToComponent(ItemRootShape, FAttachmentTransformRules::KeepRelativeTransform);
	ItemInteractionButton->AttachToComponent(ItemMesh, FAttachmentTransformRules::KeepRelativeTransform);
	ItemInteractionButtonText->AttachToComponent(ItemInteractionButton, FAttachmentTransformRules::KeepRelativeTransform);

	// 트랜스폼 및 콜리전, 피직스 등등 설정 여기에
	ItemRootShape->SetWorldTransform(CachedRootWorldTransform);
	ItemMesh->SetRelativeTransform(MetaData->MeshTransform);
	ItemInteractionButton->SetRelativeTransform(MetaData->IxButtonTransform);
	ItemInteractionButtonText->SetRelativeTransform(MetaData->IxButtonTextTransform);
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
	
	if (Execute_CanUseRootAsTriggerShape(this))
	{
		OnActorBeginOverlap.AddDynamic(this, &ThisClass::NotifyInteractableFocusBegin);
		OnActorEndOverlap.AddDynamic(this, &ThisClass::NotifyInteractableFocusEnd);
	}
}

const UNAItemData* ANAItemActor::GetItemData() const
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

bool ANAItemActor::CanUseRootAsTriggerShape_Implementation() const
{
	return ItemRootShape && (ItemRootShape->GetBodySetup() ? true : false);
}

bool ANAItemActor::CanInteract_Implementation() const
{
	return bIsFocused && Execute_CanUseRootAsTriggerShape(this) && Execute_GetInteractableData(this).InteractingCharacter.IsValid();
}

void ANAItemActor::NotifyInteractableFocusBegin_Implementation(AActor* InteractableActor, AActor* InteractorActor)
{
	bIsFocused = true;
	
	if (Execute_CanInteract(this))
	{
		if (UNAInteractionComponent* InteractionComp = TryGetInteractionComponent(InteractorActor))
		{
			//InteractionComp->OnInteractableFound(this);
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
			//InteractionComp->OnInteractableLost(this);
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
	//if (UNAInteractionComponent* InteractionComp = TryGetInteractionComponent(InteractorActor))
	//{
	bIsOnInteract = false;

	// @TODO: 상호작용 종료 시 필요한 로직이 있으면 여기에 추가, 상호작용 종료를 알리는 이벤트라고 생각하면 됨
	//}
}

void ANAItemActor::ExecuteInteract_Implementation(AActor* InteractorActor)
{
	ensureAlwaysMsgf(bIsOnInteract, TEXT("[INAInteractableInterface::ExecuteInteract_Implementation]  bIsOnInteract이 false였음"));
	
	//if (UNAInteractionComponent* InteractionComp = TryGetInteractionComponent(InteractorActor))
	//{

	// @TODO: 상호작용 실행에 필요한 로직이 있으면 여기에 추가
	//}
	
}

bool ANAItemActor::IsOnInteract_Implementation() const
{
	return bIsOnInteract;
}