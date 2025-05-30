
#include "Item/ItemActor/NAItemActor.h"

#include "DataTableEditorUtils.h"
#include "Item/Subsystem/NAItemEngineSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Interaction/NAInteractionComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/Button.h"
#include "Components/TextRenderComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "GeometryCollection/GeometryCollectionCache.h"


ANAItemActor::ANAItemActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (HasAnyFlags(RF_ClassDefaultObject) && !GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s]  c++ CDO 생성자"), *GetClass()->GetName());
	}
	else if (HasAnyFlags(RF_ClassDefaultObject) && GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s]  블프 CDO 생성자"), *GetClass()->GetName());
	}
	else if (!HasAnyFlags(RF_ClassDefaultObject) && !GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s]  c++ 일반 객체 생성자"), *GetClass()->GetName());
		// ANAItemActor 계열 c++ 네이티브 클래스들에 Abstract 꼭 붙여놓을 것. 여기에 걸리면 안됨
	}
	else if (!HasAnyFlags(RF_ClassDefaultObject) && GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s]  블프 일반 객체 생성자"), *GetClass()->GetName());
	}
	
	if (IsValid(UNAItemEngineSubsystem::Get()) && UNAItemEngineSubsystem::Get()->IsItemMetaDataInitialized())
	{
		const FNAItemBaseTableRow* RowData = UNAItemEngineSubsystem::Get()->GetItemMetaData(GetClass());
		if (RowData)
		{
			switch (RowData->RootShapeType)
			{
			case EItemRootShapeType::IRT_Sphere:
				ItemRootShape = CreateDefaultSubobject<USphereComponent>(TEXT("ItemRootShape(Sphere)"));
				break;
			case EItemRootShapeType::IRT_Box:
				ItemRootShape = CreateDefaultSubobject<UBoxComponent>(TEXT("ItemRootShape(Box)"));
				break;
			case EItemRootShapeType::IRT_Capsule:
				ItemRootShape = CreateDefaultSubobject<UCapsuleComponent>(TEXT("ItemRootShape(Capsule)"));
				break;

			default:
				ensureAlwaysMsgf(false, TEXT("dd" ));
				break;
			}
			ItemRootShapeType = RowData->RootShapeType;
			USceneComponent* OldRoot = GetRootComponent();
			SetRootComponent(ItemRootShape);
			if (USphereComponent* RootSphere = Cast<USphereComponent>(ItemRootShape))
			{
				RootSphere->SetSphereRadius(RowData->CachedTransforms.RootSphereRadius);
			}
			else if (UBoxComponent* RootBox = Cast<UBoxComponent>(ItemRootShape))
			{
				RootBox->SetBoxExtent(RowData->CachedTransforms.RootBoxExtent);
			}
			else if (UCapsuleComponent* RootCapsule = Cast<UCapsuleComponent>(ItemRootShape))
			{
				RootCapsule->SetCapsuleSize(RowData->CachedTransforms.RootCapsuleSize.X, RowData->CachedTransforms.RootCapsuleSize.Y);
			}
			if (OldRoot)
			{
				OldRoot->DestroyComponent();
				OldRoot->RemoveFromRoot();
				OldRoot = nullptr;
			}

			switch (RowData->MeshType)
			{
			case EItemMeshType::IMT_Static:
				ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh(Static)"));
				break;
			case EItemMeshType::IMT_Skeletal:
				ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh(Skeletal)"));
				break;

			default:
				ensureAlwaysMsgf(false, TEXT("ddd" ));
				break;
			}
			ItemMeshType = RowData->MeshType;
			ItemMesh->SetupAttachment(ItemRootShape);
			ItemMesh->SetRelativeTransform(RowData->CachedTransforms.MeshTransform);
		
			ItemInteractionButton = CreateDefaultSubobject<UBillboardComponent>(TEXT("ItemInteractionButton"));
			ItemInteractionButton->SetupAttachment(ItemRootShape);
			ItemInteractionButtonText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ItemInteractionButtonText"));
			ItemInteractionButtonText->SetupAttachment(ItemInteractionButton);
#if WITH_EDITOR
			if (!HasAnyFlags(RF_ClassDefaultObject)
				&& !UNAItemEngineSubsystem::Get()->OnItemActorCDOPatched.IsBoundToObject(this))
			{
				UNAItemEngineSubsystem::Get()->OnItemActorCDOPatched.AddUObject(this, &ANAItemActor::ExecuteItemPatch);
			}
#endif
		}
	}
	
	ItemDataID = NAME_None;
	bItemDataInitialized = false;
	
}

void ANAItemActor::PostInitProperties()
{
	Super::PostInitProperties();
	
	if (HasAnyFlags(RF_ClassDefaultObject) && !GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s]  c++ CDO PostInitProperties"), *GetClass()->GetName());
	}
	else if (HasAnyFlags(RF_ClassDefaultObject) && GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s]  블프 CDO PostInitProperties"), *GetClass()->GetName());
		// 이 함수 끝나고 블프 CDO 직렬화 시작
	}
	else if (!HasAnyFlags(RF_ClassDefaultObject) && !GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s]  c++ 일반 객체 PostInitProperties"), *GetClass()->GetName());
	}
	else if (!HasAnyFlags(RF_ClassDefaultObject) && GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s]  블프 일반 객체 PostInitProperties"), *GetClass()->GetName());
	}
}

// 이거 끝나고 직렬화
void ANAItemActor::PostLoad()
{
	Super::PostLoad();
	
	if (HasAnyFlags(RF_ClassDefaultObject) && !GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s]  c++ CDO PostLoad"), *GetClass()->GetName());
	}
	else if (HasAnyFlags(RF_ClassDefaultObject) && GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s]  블프 CDO PostLoad"), *GetClass()->GetName());
	}
	else if (!HasAnyFlags(RF_ClassDefaultObject) && !GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s]  c++ 일반 객체 PostLoad"), *GetClass()->GetName());
	}
	else if (!HasAnyFlags(RF_ClassDefaultObject) && GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s]  블프 일반 객체 PostLoad"), *GetClass()->GetName());
	}
}

void ANAItemActor::PostLoadSubobjects(FObjectInstancingGraph* OuterInstanceGraph)
{
	Super::PostLoadSubobjects(OuterInstanceGraph);

	if (HasAnyFlags(RF_ClassDefaultObject) && GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s]  블프 CDO PostLoadSubobjects"), *GetClass()->GetName());
	}
}

void ANAItemActor::PostActorCreated()
{
	Super::PostActorCreated();
	
	if (HasAnyFlags(RF_ClassDefaultObject) && GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s]  블프 CDO PostActorCreated"), *GetClass()->GetName());
	}
	else if (!HasAnyFlags(RF_ClassDefaultObject) && GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s]  블프 일반 객체 PostActorCreated"), *GetClass()->GetName());
	}
}

void ANAItemActor::PreDuplicate(FObjectDuplicationParameters& DupParams)
{
	Super::PreDuplicate(DupParams);
	
	if (HasAnyFlags(RF_ClassDefaultObject) && GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s]  블프 CDO PreDuplicate"), *GetClass()->GetName());
	}
	else if (!HasAnyFlags(RF_ClassDefaultObject) && GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s]  블프 일반 객체 PreDuplicate"), *GetClass()->GetName());
	}
}

void ANAItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (!HasAnyFlags(RF_ClassDefaultObject) && !GetWorld()->IsPreviewWorld())
	{
		if (ItemDataID.IsNone())
		{
			const UNAItemData* ItemData = UNAItemEngineSubsystem::Get()->CreateItemDataByActor(this);
			ItemDataID = ItemData->GetItemID();
			
			bItemDataInitialized = true;
		}
		else
		{
			bItemDataInitialized = true;
		}
	}
}

#if WITH_EDITOR
void ANAItemActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (FNAItemBaseTableRow* ItemDataStruct = UNAItemEngineSubsystem::Get()->AccessItemMetaData(GetClass()))
	{
		if (UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(ItemMesh))
		{
			if (StaticMesh->GetStaticMesh() != ItemDataStruct->StaticMeshAssetData.StaticMesh)
			{
				ItemDataStruct->StaticMeshAssetData.StaticMesh = StaticMesh->GetStaticMesh();
			}
		}
		else if (USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(ItemMesh))
		{
			if (SkeletalMesh->GetSkeletalMeshAsset() != ItemDataStruct->SkeletalMeshAssetData.SkeletalMesh)
			{
				ItemDataStruct->SkeletalMeshAssetData.SkeletalMesh = SkeletalMesh->GetSkeletalMeshAsset();
			}
		}

		if (InteractableInterfaceRef)
		{
			FText meta_InteractionName = InteractableInterfaceRef->GetInteractableData_Internal().InteractionName;
			FText this_InteractionName = ItemInteractionButtonText->Text;
			if (this_InteractionName.ToString() != meta_InteractionName.ToString())
			{
				meta_InteractionName = this_InteractionName;
			}
		}

		if (ItemInteractionButton->Sprite.Get() != ItemDataStruct->IconAssetData.Icon)
		{
			ItemDataStruct->IconAssetData.Icon = ItemInteractionButton->Sprite.Get();
		}
	}
}

void ANAItemActor::ExecuteItemPatch(UClass* ClassToPatch, const FNAItemBaseTableRow* PatchData, EItemMetaDirtyFlags PatchFlags)
{
	if (HasAnyFlags(RF_ClassDefaultObject) || !GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		return;
	}

	if ( GetClass()->ClassGeneratedBy == ClassToPatch->ClassGeneratedBy && PatchData)
	{
		const EObjectFlags SubobjFlags = GetMaskedFlags(RF_PropagateToSubObjects) | RF_DefaultSubObject;

		TArray<UActorComponent*> OldComponents;

		// Create new subobjects
		FTransform CachedRootWorldTransform = FTransform::Identity;
		if (EnumHasAnyFlags(PatchFlags, EItemMetaDirtyFlags::MF_RootShape))
		{
			if (ItemRootShape)
			{
				OldComponents.Emplace(ItemRootShape.Get());
				
				if (!GetWorld()->IsPreviewWorld())
				{
					CachedRootWorldTransform = ItemRootShape->GetComponentTransform();
				}
			}
			
			switch (PatchData->RootShapeType)
			{
			case EItemRootShapeType::IRT_Sphere:
				ItemRootShape = NewObject<USphereComponent>(
					this, USphereComponent::StaticClass(),TEXT("ItemRootShape(Sphere)"), SubobjFlags);
				break;
			case EItemRootShapeType::IRT_Box:
				ItemRootShape = NewObject<UBoxComponent>(
					this, UBoxComponent::StaticClass(),TEXT("ItemRootShape(Box)"), SubobjFlags);
				break;
			case EItemRootShapeType::IRT_Capsule:
				ItemRootShape = NewObject<UCapsuleComponent>(
					this, UCapsuleComponent::StaticClass(),TEXT("ItemRootShape(Capsule)"),
					SubobjFlags);
				break;

			default:
				ensureAlwaysMsgf(false, TEXT("ttt" ));
				break;
			}
			if (ItemRootShape == nullptr)
			{
				ensureAlwaysMsgf(false, TEXT("ttt"));
				return;
			}
			ItemRootShapeType = PatchData->RootShapeType;
			if (USphereComponent* RootSphere = Cast<USphereComponent>(ItemRootShape))
			{
				RootSphere->SetSphereRadius(PatchData->CachedTransforms.RootSphereRadius);
			}
			else if (UBoxComponent* RootBox = Cast<UBoxComponent>(ItemRootShape))
			{
				RootBox->SetBoxExtent(PatchData->CachedTransforms.RootBoxExtent);
			}
			else if (UCapsuleComponent* RootCapsule = Cast<UCapsuleComponent>(ItemRootShape))
			{
				RootCapsule->SetCapsuleSize(PatchData->CachedTransforms.RootCapsuleSize.X, PatchData->CachedTransforms.RootCapsuleSize.Y);
			}
			ItemRootShape->CreationMethod = EComponentCreationMethod::Native;
			AddOwnedComponent(ItemRootShape);
			AddInstanceComponent(ItemRootShape);
		}

		FTransform CachedMeshRelativeTransform = FTransform::Identity;
		if (EnumHasAnyFlags(PatchFlags, EItemMetaDirtyFlags::MF_Mesh))
		{
			if (ItemMesh)
			{
				OldComponents.Emplace(ItemMesh.Get());
				CachedMeshRelativeTransform = ItemMesh->GetRelativeTransform();
			}

			switch (PatchData->MeshType)
			{
			case EItemMeshType::IMT_Static:
				ItemMesh = NewObject<UStaticMeshComponent>(
					this, UStaticMeshComponent::StaticClass(),TEXT("ItemMesh(Static)"), SubobjFlags);
				break;
			case EItemMeshType::IMT_Skeletal:
				ItemMesh = NewObject<USkeletalMeshComponent>(
					this, USkeletalMeshComponent::StaticClass(),TEXT("ItemMesh(Skeletal)"), SubobjFlags);
				break;

			default:
				ensureAlwaysMsgf(false,
				                 TEXT( "yy"));
				break;
			}
			if (ItemMesh == nullptr)
			{
				ensureAlwaysMsgf(false, TEXT("yy"));
				return;
			}
			ItemMeshType = PatchData->MeshType;
			ItemMesh->CreationMethod = EComponentCreationMethod::Native;
			AddOwnedComponent(ItemMesh);
			AddInstanceComponent(ItemRootShape);
		}

		FTransform CachedButtonRelativeTransform = FTransform::Identity;
		if (EnumHasAnyFlags(PatchFlags, EItemMetaDirtyFlags::MF_IxButton))
		{
			if (ItemInteractionButton)
			{
				OldComponents.Emplace(ItemInteractionButton.Get());
				CachedButtonRelativeTransform = ItemInteractionButton->GetRelativeTransform();
			}

			ItemInteractionButton = NewObject<UBillboardComponent>(
				this, UBillboardComponent::StaticClass(),TEXT("ItemInteractionButton"), SubobjFlags);

			if (ItemInteractionButton == nullptr)
			{
				ensureAlwaysMsgf(false,
				                 TEXT(
					                 "dd"));
				return;
			}
			ItemInteractionButton->CreationMethod = EComponentCreationMethod::Native;
			AddOwnedComponent(ItemInteractionButton);
			AddInstanceComponent(ItemInteractionButton);
		}

		FTransform CachedButtonTextRelativeTransform = FTransform::Identity;
		if (EnumHasAnyFlags(PatchFlags, EItemMetaDirtyFlags::MF_IxButton))
		{
			if (ItemInteractionButtonText)
			{
				OldComponents.Emplace(ItemInteractionButtonText.Get());
				CachedButtonTextRelativeTransform = ItemInteractionButtonText->GetRelativeTransform();
			}

			ItemInteractionButtonText = NewObject<UTextRenderComponent>(
				this, UTextRenderComponent::StaticClass(),TEXT("ItemInteractionButtonText"), SubobjFlags);

			if (ItemInteractionButtonText == nullptr)
			{
				ensureAlwaysMsgf(false,
				                 TEXT(
					                 "dde"));
				return;
			}
			ItemInteractionButtonText->CreationMethod = EComponentCreationMethod::Native;
			AddOwnedComponent(ItemInteractionButtonText);
			AddInstanceComponent(ItemInteractionButtonText);
		}

		// Hierarchy & Property settings
		SetRootComponent(ItemRootShape);
		ItemRootShape->RegisterComponent();
		ItemRootShape->Mobility = EComponentMobility::Movable;

		ItemMesh->AttachToComponent(ItemRootShape, FAttachmentTransformRules::KeepRelativeTransform);
		ItemMesh->RegisterComponent();
		ItemMesh->Mobility = EComponentMobility::Movable;
		if (UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(ItemMesh))
		{
			StaticMeshComp->SetStaticMesh(PatchData->StaticMeshAssetData.StaticMesh);

			ItemFractureCollection = PatchData->StaticMeshAssetData.FractureCollection;
			ItemFractureCache = PatchData->StaticMeshAssetData.FractureCache;
		}
		else if (USkeletalMeshComponent* SkeletalMeshComp = Cast<USkeletalMeshComponent>(ItemMesh))
		{
			SkeletalMeshComp->SetSkeletalMesh(PatchData->SkeletalMeshAssetData.SkeletalMesh);
			SkeletalMeshComp->SetAnimClass(PatchData->SkeletalMeshAssetData.AnimClass.Get());
		}

		ItemInteractionButton->AttachToComponent(ItemMesh, FAttachmentTransformRules::KeepRelativeTransform);
		ItemInteractionButton->RegisterComponent();
		ItemInteractionButton->SetSprite(PatchData->IconAssetData.Icon);

		ItemInteractionButtonText->AttachToComponent(ItemInteractionButton, FAttachmentTransformRules::KeepRelativeTransform);
		ItemInteractionButtonText->RegisterComponent();

		if (!OldComponents.IsEmpty())
		{
			for (UActorComponent* OldComponent : OldComponents)
			{
				OldComponent->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors);
				OldComponent->DestroyComponent();
				OldComponent = nullptr;
			}
			OldComponents.Empty();
		}

		if (!CachedRootWorldTransform.Equals(FTransform::Identity))
		{
			ItemRootShape->SetWorldTransform(CachedRootWorldTransform);
		}
		
		const FTransform NewMeshRelativeTransform =
			CachedMeshRelativeTransform.Equals(PatchData->CachedTransforms.MeshTransform)
				? CachedMeshRelativeTransform
				: PatchData->CachedTransforms.MeshTransform;
		ItemMesh->SetRelativeTransform(NewMeshRelativeTransform);

		const FTransform NewButtonRelativeTransform =
			CachedButtonRelativeTransform.Equals(PatchData->CachedTransforms.ButtonTransform)
				? CachedButtonRelativeTransform
				: PatchData->CachedTransforms.ButtonTransform;
		ItemInteractionButton->SetRelativeTransform(NewButtonRelativeTransform);
		
		const FTransform NewButtonTextRelativeTransform =
			NewButtonTextRelativeTransform.Equals(PatchData->CachedTransforms.ButtonTextTransform)
				? NewButtonTextRelativeTransform
				: PatchData->CachedTransforms.ButtonTextTransform;
		ItemInteractionButtonText->SetRelativeTransform(NewButtonTextRelativeTransform);
	}
}
#endif

void ANAItemActor::BeginItemInitialize_Internal()
{
	InitItemData_Internal();
	InitItemActor_Internal();
}

void ANAItemActor::InitItemData_Internal()
{
	if (bItemDataInitialized)
	{
		return;
	}

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		if (!ItemDataID.IsNone())
		{
			bItemDataInitialized = true;
			VerifyInteractableData_Internal();
			return;
		}
		else
		{
			ensure(false);
			return;
		}
	}

	if (const UNAItemData* NewItemData = UNAItemEngineSubsystem::Get()->CreateItemDataByActor(this))
	{
		ItemDataID = NewItemData->GetItemID();
		bItemDataInitialized = true;
		OnItemDataInitialized();
	}
}

void ANAItemActor::OnItemDataInitialized_Implementation()
{
	VerifyInteractableData_Internal();
}

void ANAItemActor::VerifyInteractableData_Internal()
{
	if (InteractableInterfaceRef != nullptr)
	{
		return;
	}
	
	// 이 액터가 UNAInteractableInterface 인터페이스를 구현했다면 this를 할당
	if (bItemDataInitialized && GetClass()->ImplementsInterface(UNAInteractableInterface::StaticClass()))
	{
		InteractableInterfaceRef = this;
		
		if (!CheckInteractableEdit(InteractableInterfaceRef->Execute_GetInteractableData(this)))
		{
			ensure(false);
		}
	}
	else
	{
		ensure(false);
	}
}

void ANAItemActor::InitItemActor_Internal()
{
	if (bItemDataInitialized)
	{
		InitItemActor_Impl();
		OnItemActorInitialized();
	}
}

void ANAItemActor::OnItemActorInitialized_Implementation()
{
}

void ANAItemActor::InitItemActor_Impl()
{
	// OnConstruction 때 뭐할거임
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

//======================================================================================================================
// Interactable Interface Implements
//======================================================================================================================

FNAInteractableData ANAItemActor::GetInteractableData_Implementation() const
{
	return GetInteractableData_Internal();
}

const FNAInteractableData& ANAItemActor::GetInteractableData_Internal() const
{
	const FNAInteractableData* InteractableDataRef = nullptr;
	//if (ItemData.IsValid())
	if (GEngine)
	{
		// const FNAItemBaseTableRow* ItemMetaData = ItemData->GetItemMetaDataStruct<FNAItemBaseTableRow>();
		// if (ItemMetaData)
		// {
		// 	InteractableDataRef = TryGetInteractableData(ItemMetaData);
		// }
		const UNAItemData* ThisItemData = UNAItemEngineSubsystem::Get()->GetRuntimeItemData(ItemDataID);
		if (ThisItemData)
		{
			if (const FNAItemBaseTableRow* ItemMetaData = ThisItemData->GetItemMetaDataStruct<FNAItemBaseTableRow>())
			{
				InteractableDataRef = TryGetInteractableData(ItemMetaData);
			}
		}
	}
	
	return *InteractableDataRef;
}

void ANAItemActor::SetInteractableData_Implementation(const FNAInteractableData& NewInteractableData)
{
	if (GetItemData())
	{
		FNAItemBaseTableRow* ItemMetaData = GetItemData()->ItemMetaDataHandle.GetRow<FNAItemBaseTableRow>(GetItemData()->ItemMetaDataHandle.RowName.ToString());
		if (ItemMetaData)
		{
			TrySetInteractableData(ItemMetaData, NewInteractableData);
		}
	}
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

void ANAItemActor::GetInitInteractableDataParams_MacroHook(ENAInteractableType& OutInteractableType, FString& OutInteractableName,
	/*FString& OutInteractableScript,*/ float& OutInteractableDuration, int32& OutQuantity) const
{
	UE_LOG(LogTemp, Warning, TEXT("[%s::GetInitInteractableDataParams_MacroHook]  오버라이딩 없음. ANAItemActor의 기본값으로 초기화"), *GetClass()->GetName());
	
	OutInteractableType		= ENAInteractableType::None;
	OutInteractableName		= TEXT("");
	/*OutInteractableScript	= TEXT("");*/
	OutInteractableDuration = 0.f;
	OutQuantity				= 0;
}

void ANAItemActor::SetInteractableDataToBaseline_Implementation(ENAInteractableType InInteractableType,
                                                          const FString& InInteractionName, /*const FString& InInteractionScript,*/ float InInteractionDuration, int32 InQuantity)
{
	// if (!this->GetClass()->IsChildOf<class ANAPickableItemActor>())
	// {
	// 	InQuantity = FMath::Clamp(InQuantity, 0, 1);
	// }
	//
	FNAInteractableData InteractableDataBaseline;
	InteractableDataBaseline.InteractableType = InInteractableType;
	InteractableDataBaseline.InteractionName = FText::FromString(InInteractionName);
	//InteractableDataBaseline.InteractionScript = FText::FromString(InInteractionScript);
	InteractableDataBaseline.InteractionDuration = InInteractionDuration;
	InteractableDataBaseline.Quantity = InQuantity;
	MarkBaselineModified(InteractableDataBaseline);

	Execute_SetInteractableData(this, InteractableDataBaseline);
}

bool ANAItemActor::IsOnInteract_Implementation() const
{
	return bIsOnInteract;
}