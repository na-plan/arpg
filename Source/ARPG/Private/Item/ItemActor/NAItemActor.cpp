
#include "Item/ItemActor/NAItemActor.h"

#include "Item/EngineSubsystem/NAItemEngineSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Interaction/NAInteractionComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/TextRenderComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"

#if WITH_EDITOR
#define TRANSFORM_OVERRIDE_BY_CACHE( Component, PreviousTransform, CacheTransform ) \
	{\
		const FTransform UpdateTransform =\
					PreviousTransform.Equals(CacheTransform)\
						? PreviousTransform : CacheTransform;\
		Component->SetRelativeTransform(UpdateTransform);\
	}

void FMeshUpdatePredication::operator()( AActor* InOuter, UMeshComponent* InComponent, UMeshComponent* InOldComponent,
	const FNAItemBaseTableRow* InRow, const EItemMetaDirtyFlags DirtyFlags ) const
{
	if ( ANAItemActor* BPCDO = Cast<ANAItemActor>( InOuter ) )
	{
		BPCDO->ItemMeshType = InRow->MeshType;
		InComponent->SetupAttachment(BPCDO->ItemRootShape);
		InComponent->Mobility = EComponentMobility::Movable;
		InComponent->SetRelativeTransform( InRow->CachedTransforms.MeshTransform );

		if (UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>( InComponent ))
		{
			if (EnumHasAnyFlags(DirtyFlags, EItemMetaDirtyFlags::MF_Mesh | EItemMetaDirtyFlags::MF_MeshAsset))
			{
				StaticMeshComp->SetStaticMesh(InRow->StaticMeshAssetData.StaticMesh);
			}

			BPCDO->ItemFractureCollection = InRow->StaticMeshAssetData.FractureCollection;
			BPCDO->ItemFractureCache = InRow->StaticMeshAssetData.FractureCache;
		}
		else if ( USkeletalMeshComponent* SkeletalMeshComp = Cast<USkeletalMeshComponent>( InComponent ) )
		{
			if (EnumHasAnyFlags(DirtyFlags, EItemMetaDirtyFlags::MF_Mesh))
			{
				SkeletalMeshComp->SetSkeletalMesh(InRow->SkeletalMeshAssetData.SkeletalMesh);
				SkeletalMeshComp->SetAnimClass(InRow->SkeletalMeshAssetData.AnimClass.Get());
			}
			else if (EnumHasAnyFlags(DirtyFlags, EItemMetaDirtyFlags::MF_MeshAsset))
			{
				SkeletalMeshComp->SetSkeletalMesh(InRow->SkeletalMeshAssetData.SkeletalMesh);
			}
			else if (EnumHasAnyFlags(DirtyFlags, EItemMetaDirtyFlags::MF_MeshAnim))
			{
				SkeletalMeshComp->SetAnimClass(InRow->SkeletalMeshAssetData.AnimClass.Get());
			}
		}
	}
}

void FMeshInstanceUpdatePredication::operator()( AActor* InOuter, UMeshComponent* InComponent,
	UMeshComponent* InOldComponent, const FNAItemBaseTableRow* InRow, const EItemMetaDirtyFlags DirtyFlags ) const
{
	FMeshUpdatePredication::operator()( InOuter, InComponent, InOldComponent, InRow, DirtyFlags );

	FTransform CachedMeshRelativeTransform = FTransform::Identity;
	if ( InOldComponent )
	{
		CachedMeshRelativeTransform = InOldComponent->GetRelativeTransform();
	}
	
	TRANSFORM_OVERRIDE_BY_CACHE( InComponent, CachedMeshRelativeTransform, InRow->CachedTransforms.MeshTransform );
}

void FRootShapeUpdatePredication::operator()( AActor* InOuter, UShapeComponent* InComponent,
                                              UShapeComponent* InOldComponent, const FNAItemBaseTableRow* InRow, const EItemMetaDirtyFlags DirtyFlags ) const
{
	if ( ANAItemActor* BPCDO = Cast<ANAItemActor>( InOuter ) )
	{
		BPCDO->ItemRootShapeType = InRow->RootShapeType;
		BPCDO->SetRootComponent( InComponent );
		InComponent->SetWorldTransform( InRow->CachedTransforms.RootTransform );

		if ( USphereComponent* RootSphere = Cast<USphereComponent>( InComponent ) )
		{
			RootSphere->SetSphereRadius( InRow->CachedTransforms.RootSphereRadius );
		}
		else if ( UBoxComponent* RootBox = Cast<UBoxComponent>( InComponent ) )
		{
			RootBox->SetBoxExtent( InRow->CachedTransforms.RootBoxExtent );
		}
		else if ( UCapsuleComponent* RootCapsule = Cast<UCapsuleComponent>( InComponent ) )
		{
			RootCapsule->SetCapsuleSize( InRow->CachedTransforms.RootCapsuleSize.X, InRow->CachedTransforms.RootCapsuleSize.Y );
		}
	}
}

void FRootShapeInstanceUpdatePredication::operator()( AActor* InOuter, UShapeComponent* InComponent,
	UShapeComponent* InOldComponent, const FNAItemBaseTableRow* InRow, const EItemMetaDirtyFlags DirtyFlags ) const
{
	FRootShapeUpdatePredication::operator()( InOuter, InComponent, InOldComponent, InRow, DirtyFlags );

	FTransform CachedRootWorldTransform = FTransform::Identity;
	
	if ( !InOuter->GetWorld()->IsPreviewWorld() )
	{
		CachedRootWorldTransform = InOldComponent->GetComponentTransform();
	}

	{
		const FTransform UpdateTransform = CachedRootWorldTransform.Equals( InRow->CachedTransforms.RootTransform )
			                                   ? CachedRootWorldTransform
			                                   : InRow->CachedTransforms.RootTransform;
		InComponent->SetWorldTransform( UpdateTransform );
	};
	
	if (USphereComponent* RootSphere = Cast<USphereComponent>(InComponent))
	{
		RootSphere->SetSphereRadius( InRow->CachedTransforms.RootSphereRadius );
	}
	else if (UBoxComponent* RootBox = Cast<UBoxComponent>(InComponent))
	{
		RootBox->SetBoxExtent( InRow->CachedTransforms.RootBoxExtent );
	}
	else if (UCapsuleComponent* RootCapsule = Cast<UCapsuleComponent>(InComponent))
	{
		RootCapsule->SetCapsuleSize( InRow->CachedTransforms.RootCapsuleSize.X, InRow->CachedTransforms.RootCapsuleSize.Y );
	}
}

void FInteractionButtonUpdatePredication::operator()( AActor* InOuter, UBillboardComponent* InComponent,
                                                      UBillboardComponent* InOldComponent, const FNAItemBaseTableRow* InRow, const EItemMetaDirtyFlags DirtyFlags ) const
{
	if ( const ANAItemActor* ItemCDO = Cast<ANAItemActor>( InOuter );
		 InComponent && ItemCDO )
	{
		InComponent->SetupAttachment( ItemCDO->ItemMesh );
		InComponent->SetSprite( InRow->IconAssetData.Icon );
	}	
}

void FInteractionButtonInstanceUpdatePredication::operator()( AActor* InOuter, UBillboardComponent* InComponent,
	UBillboardComponent* InOldComponent, const FNAItemBaseTableRow* InRow, const EItemMetaDirtyFlags DirtyFlags ) const
{
	FInteractionButtonUpdatePredication::operator()( InOuter, InComponent, InOldComponent, InRow, DirtyFlags );

	FTransform CachedButtonRelativeTransform = FTransform::Identity;
	if ( InOldComponent )
	{
		CachedButtonRelativeTransform = InOldComponent->GetComponentTransform();
	}
	
	TRANSFORM_OVERRIDE_BY_CACHE( InComponent, CachedButtonRelativeTransform, InRow->CachedTransforms.ButtonTransform );
}

void FInteractionButtonTextUpdatePredication::operator()( AActor* InOuter, UTextRenderComponent* InComponent,
                                                          UTextRenderComponent* InOldComponent, const FNAItemBaseTableRow* InRow, const EItemMetaDirtyFlags DirtyFlags ) const
{
	if ( ANAItemActor* ItemCDO = Cast<ANAItemActor>( InOuter ) )
	{
		InComponent->SetupAttachment( ItemCDO->ItemInteractionButton );
	}
}

void FInteractionButtonTextInstanceUpdatePredication::operator()( AActor* InOuter, UTextRenderComponent* InComponent,
	UTextRenderComponent* InOldComponent, const FNAItemBaseTableRow* InRow, const EItemMetaDirtyFlags DirtyFlags ) const
{
	FInteractionButtonTextUpdatePredication::operator()( InOuter, InComponent, InOldComponent, InRow, DirtyFlags );

	FTransform CachedButtonTextRelativeTransform = FTransform::Identity;
	if ( InOldComponent )
	{
		CachedButtonTextRelativeTransform = InOldComponent->GetComponentTransform();
	}

	TRANSFORM_OVERRIDE_BY_CACHE( InComponent, CachedButtonTextRelativeTransform, InRow->CachedTransforms.ButtonTextTransform );
}

UShapeComponent* FRootShapeSpawnPredication::operator()( UObject* InOuter, const FName& InComponentName,
                                                         const EObjectFlags InObjectFlags, const FNAItemBaseTableRow* InRow ) const
{
	UShapeComponent* Result = nullptr;
			
	switch ( InRow->RootShapeType )
	{
	case EItemRootShapeType::IRT_Sphere:
		Result = NewObject<USphereComponent>(
			InOuter, USphereComponent::StaticClass(),TEXT("ItemRootShape(Sphere)"), InObjectFlags);
		break;
	case EItemRootShapeType::IRT_Box:
		Result = NewObject<UBoxComponent>(
			InOuter, UBoxComponent::StaticClass(),TEXT("ItemRootShape(Box)"), InObjectFlags);
		break;
	case EItemRootShapeType::IRT_Capsule:
		Result = NewObject<UCapsuleComponent>(
			InOuter, UCapsuleComponent::StaticClass(),TEXT("ItemRootShape(Capsule)"),
			InObjectFlags);
		break;
	default:
		ensureAlwaysMsgf( false, TEXT("%hs: RootShapeType이 none이었음. ???"), __FUNCTION__ );
	}
			
	if (Result == nullptr)
	{
		ensureAlwaysMsgf( false, TEXT( "%hs: 생성 실패. ???"), __FUNCTION__ );
	}

	return Result;
}

UMeshComponent* FMeshSpawnPredication::operator()( UObject* InOuter, const FName& InComponentName,
                                                   const EObjectFlags InObjectFlags, const FNAItemBaseTableRow* InRow ) const
{
	UMeshComponent* Result = nullptr;

	switch ( InRow->MeshType )
	{
	case EItemMeshType::IMT_Static:
		Result = NewObject<UStaticMeshComponent>(
			InOuter, UStaticMeshComponent::StaticClass(),TEXT("ItemMesh(Static)"), InObjectFlags);
		break;
	case EItemMeshType::IMT_Skeletal:
		Result = NewObject<USkeletalMeshComponent>(
			InOuter, USkeletalMeshComponent::StaticClass(),TEXT("ItemMesh(Skeletal)"),
			InObjectFlags);
		break;
	default:
		ensureAlwaysMsgf( false, TEXT( "%hs: MeshType none이었음. ???") , __FUNCTION__ );
	}
	
	if ( Result == nullptr )
	{
		ensureAlwaysMsgf( false, TEXT("%hs: 생성 실패. ???"), __FUNCTION__ );
	}

	return Result;
}
#endif

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
			FNAInteractableData IxData;
			if (GetInteractableData_Internal(IxData))
			{
				FText meta_InteractionName = IxData.InteractionName;
				FText this_InteractionName = ItemInteractionButtonText->Text;
				if (this_InteractionName.ToString() != meta_InteractionName.ToString())
				{
					meta_InteractionName = this_InteractionName;
				}
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
		FItemPatchHelper::UpdateDirtyComponent<
			decltype(ItemRootShape)::ElementType,
			EItemMetaDirtyFlags::MF_RootShape,
			FRootShapeSpawnPredication,
			FRootShapeInstanceUpdatePredication>
		(
			PatchFlags,
			OldComponents,
			this,
			&ItemRootShape,
			PatchData,
			TEXT( "" ),
			SubobjFlags,
			true
		);

		FItemPatchHelper::UpdateDirtyComponent<
			decltype(ItemMesh)::ElementType,
			EItemMetaDirtyFlags::MF_Mesh,
			FMeshSpawnPredication,
			FMeshInstanceUpdatePredication>
		(
			PatchFlags,
			OldComponents,
			this,
			&ItemMesh,
			PatchData,
			TEXT( "" ),
			SubobjFlags,
			true
		);

		FItemPatchHelper::UpdateDirtyComponent<
			decltype(ItemInteractionButton)::ElementType,
			EItemMetaDirtyFlags::MF_IxButton,
			FItemPatchHelper::FDefaultSpawnPredication<decltype(ItemInteractionButton)::ElementType>,
			FInteractionButtonInstanceUpdatePredication>
		(
			PatchFlags,
			OldComponents,
			this,
			&ItemInteractionButton,
			PatchData,
			TEXT( "ItemInteractionButton" ),
			SubobjFlags,
			true
		);

		FItemPatchHelper::UpdateDirtyComponent<
			decltype(ItemInteractionButtonText)::ElementType,
			EItemMetaDirtyFlags::MF_IxButtonText,
			FItemPatchHelper::FDefaultSpawnPredication<decltype(ItemInteractionButtonText)::ElementType>,
			FInteractionButtonTextUpdatePredication>
		(
			PatchFlags,
			OldComponents,
			this,
			&ItemInteractionButtonText,
			PatchData,
			TEXT( "ItemInteractionButtonText" ),
			SubobjFlags,
			false
		);

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