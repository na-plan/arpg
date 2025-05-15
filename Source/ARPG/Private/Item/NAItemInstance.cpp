
#include "Item/NAItemInstance.h"

#include "Components/SphereComponent.h"

ANAItemInstance::ANAItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	ItemStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemStaticMesh"));
	ItemStaticMesh->SetupAttachment(RootComponent);
}

UNAItemData* ANAItemInstance::GetItemData() const
{
	return nullptr;
}

void ANAItemInstance::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetData(ItemDataTableRowHandle);
}

bool ANAItemInstance::InitItemInstance(const FDataTableRowHandle& InDataTableRowHandle)
{
	if (!IsCompatibleDataTable(InDataTableRowHandle))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s::SetData]  클래스 전용 DT를 쓰셈."), *GetClass()->GetName());
		return false;
	}

	ItemDataTableRowHandle = InDataTableRowHandle;
	FNAItemBaseTableRow* ItemData = GetItemData<FNAItemBaseTableRow>(ItemDataTableRowHandle);
	if (!ItemData) { return false;  }

	EObjectFlags SubobjectFlags = GetMaskedFlags(RF_PropagateToSubObjects) | RF_DefaultSubObject;

	if (!TriggerSphere && ItemData->bUseTriggerShpereAsRoot)
	{
		TriggerSphere = NewObject<USphereComponent>(this, USphereComponent::StaticClass(), TEXT("TriggerSphere"), SubobjectFlags);
		TriggerSphere->SetRelativeTransform(ItemData->TriggerSphereTransform);
		TriggerSphere->SetSphereRadius(ItemData->TriggerSphereRadius);
		SetRootComponent(TriggerSphere);
	}

	if (ItemData->ItemMeshType == EItemMeshType::IMT_Static)
	{
		ItemMeshClass = UStaticMeshComponent::StaticClass();
		ItemStaticMesh->SetStaticMesh(ItemData->StaticMeshItemAssetData.StaticMesh);
		ItemStaticMesh->SetRelativeTransform(ItemData->StaticMeshItemAssetData.MeshTransform);
	}
	else
	{
		ItemStaticMesh->SetVisibility(false, true);
		ItemStaticMesh->Deactivate();

		if (!ItemSkeletalMesh && ItemData->ItemMeshType == EItemMeshType::IMT_Skeletal)
		{
			ItemMeshClass = USkeletalMeshComponent::StaticClass();
			ItemSkeletalMesh = NewObject<USkeletalMeshComponent>(this, ItemMeshClass.Get(), TEXT("ItemSkeletalMesh"), SubobjectFlags);
			ItemSkeletalMesh->SetSkeletalMesh(ItemData->SkeletalMeshItemAssetData.SkeletalMesh);
			ItemSkeletalMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			ItemSkeletalMesh->SetRelativeTransform(ItemData->SkeletalMeshItemAssetData.MeshTransform);

			EWorldType::Type ThisWorldType = GetWorld()->WorldType;
			constexpr uint8 WorldTypeMask = 1 | 3 | 5 | 6; // Game, PIE, GamePrevie, GameRPC
			const bool bShouldDestroy = (static_cast<uint8>(ThisWorldType) & WorldTypeMask) != 0;
			if (bShouldDestroy)
			{
				ItemStaticMesh->DestroyComponent();
			}
			else
			{
				ItemStaticMesh->AttachToComponent(ItemSkeletalMesh, FAttachmentTransformRules::KeepRelativeTransform);
			}
		}
	}

	RegisterAllComponents();

	ItemState = ItemData->ItemState;

	return true;
}