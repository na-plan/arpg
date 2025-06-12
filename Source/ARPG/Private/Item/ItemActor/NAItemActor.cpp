
#include "Item/ItemActor/NAItemActor.h"

#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Interaction/NAInteractionComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "Item/ItemWidget/NAItemWidgetComponent.h"
#include "Physics/PhysicsFiltering.h"


ANAItemActor::ANAItemActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	
	TriggerSphere = CreateDefaultSubobject<USphereComponent>("TriggerSphere");
	TriggerSphere->SetupAttachment(RootComponent);
	TriggerSphere->SetSphereRadius(100.0f);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->CanCharacterStepUpOn = ECB_No;
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap); 
	
	ItemCollision = CreateOptionalDefaultSubobject<USphereComponent>(TEXT("ItemCollision(Sphere)"));
	ItemMesh = CreateOptionalDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh(Static)"));
	if (ItemCollision)
	{
		bUseItemCollision = true;
		ItemCollision->SetupAttachment(RootComponent);
	}
	if (ItemMesh)
	{
		bUseItemMesh = true;
		ItemMesh->SetupAttachment(ItemCollision);
	}
	
	ItemWidgetComponent = CreateDefaultSubobject<UNAItemWidgetComponent>(TEXT("ItemWidgetComponent"));
	ItemWidgetComponent->SetupAttachment(ItemCollision);
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface>
		ItemWidgetMaterial(TEXT(
			"/Script/Engine.MaterialInstanceConstant'/Engine/EngineMaterials/Widget3DPassThrough_Translucent.Widget3DPassThrough_Translucent'"));
	check(ItemWidgetMaterial.Object);
	ItemWidgetComponent->SetMaterial(0, ItemWidgetMaterial.Object);
	// @TODO 위젯 컴포넌트 트랜스폼 설정
	
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

	if (TriggerSphere->GetAttachParent() != GetRootComponent())
	{
		TriggerSphere->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	}
	
	const FNAItemBaseTableRow* MetaData = UNAItemEngineSubsystem::Get()->GetItemMetaDataByClass(GetClass());
	if (!MetaData) { return; }
	
	EItemSubobjDirtyFlags DirtyFlags = CheckDirtySubobjectFlags(MetaData);
	
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

	check(ItemCollision);
	check(ItemWidgetComponent);
	// 어태치먼트
	if (ItemCollision->GetAttachParent() != GetRootComponent())
	{
		ItemCollision->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	}
	if (ItemMesh && (ItemMesh->GetAttachParent() != ItemCollision))
	{
		ItemMesh->AttachToComponent(ItemCollision, FAttachmentTransformRules::KeepRelativeTransform);
	}
	if (ItemWidgetComponent->GetAttachParent() != ItemCollision)
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

		ItemCollision->SetRelativeTransform(MetaData->CollisionTransform);
	}

	if (MetaData->MeshType != EItemMeshType::IMT_None && ItemMesh)
	{
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
		
		ItemMesh->SetRelativeTransform(MetaData->MeshTransform);
	}

	// 트랜스폼 및 콜리전, 피직스 등등 설정 여기에
	ItemWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, TriggerSphere->GetScaledSphereRadius() * 1.5f));
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
	
	if (MetaData->CollisionShape != EItemCollisionShape::ICS_None && bUseItemCollision)
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
	
	if (MetaData->MeshType != EItemMeshType::IMT_None && bUseItemMesh)
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

	if (ItemWidgetComponent)
	{
		ItemWidgetComponent->SetVisibility(false);
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
	return IsValid(TriggerSphere) && InteractableInterfaceRef != nullptr;
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
		InteractionComp->befajfl = false;
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

void ANAItemActor::DisableOverlapDuringInteraction(AActor* Interactor)
{
	if (Execute_IsOnInteract(this))
	{
		if (UNAInteractionComponent* InteractionComp = TryGetInteractionComponent(Interactor))
		{
			TriggerSphere->SetGenerateOverlapEvents(false);
		}
	}
}
