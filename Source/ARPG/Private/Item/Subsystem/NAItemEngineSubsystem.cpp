// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Subsystem/NAItemEngineSubsystem.h"

#include "NACharacter.h"
#include "Inventory/DataStructs/NAInventoryDataStructs.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/SimpleConstructionScript.h"

#include "Item/ItemActor/NAItemActor.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/SCS_Node.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "GeometryCollection/GeometryCollectionCache.h"
#include "Kismet2/KismetEditorUtilities.h"

// 와 이것도 정적 로드로 CDO 생김 ㅁㅊ
UNAItemEngineSubsystem::UNAItemEngineSubsystem()
{
}

void UNAItemEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (ItemDataTableSources.IsEmpty())
	{
		// 1) Registry 에셋 동기 로드 (나중에 실제 경로로 교체)
		static const FString RegistryPath = TEXT("/Script/ARPG.ItemDataTablesAsset'/Game/00_ProjectNA/ItemTest/DA_ItemDataTables.DA_ItemDataTables'");
		UItemDataTablesAsset* Registry = Cast<UItemDataTablesAsset>(StaticLoadObject(UItemDataTablesAsset::StaticClass(), nullptr, *RegistryPath));

		if (!Registry)
		{
			UE_LOG(LogTemp, Error, TEXT("[UNAItemGameInstanceSubsystem::Initialize]  ItemDataTablesAsset 로드 실패: %s"), *RegistryPath);
			return;
		}

		// 2) Registry 안의 SoftObjectPtr<UDataTable> 리스트 순회
		UE_LOG(LogTemp, Warning, TEXT("[UNAItemGameInstanceSubsystem::Initialize]  아이템 DT LoadSynchronous 시작"));
		for (const TSoftObjectPtr<UDataTable>& SoftDT : Registry->ItemDataTables)
		{
			UDataTable* ResourceDT = SoftDT.LoadSynchronous(); // 이때 DT 안에 있던 BP 클래스의 CDO가 생성됨(직렬화까지 완료)
			if (!ResourceDT)
			{
				UE_LOG(LogTemp, Warning,
					TEXT("[UNAItemGameInstanceSubsystem]  Failed to load DataTable: %s"), *SoftDT.ToString());
				continue;
			}

			ItemDataTableSources.Emplace(ResourceDT);
			UE_LOG(LogTemp, Log,
				TEXT("[UNAItemGameInstanceSubsystem]  Loaded DataTable: %s"), *ResourceDT->GetName());
		}

		// (2) 메타데이터 맵 빌드
		if (ItemMetaDataMap.IsEmpty())
		{
			// ItemMetaDataMap 버킷 확보
			int32 ExpectedCount = 0;
			for (UDataTable* DT : ItemDataTableSources)
			{
				ExpectedCount += DT->GetRowMap().Num();
			}
			ItemMetaDataMap.Reserve(ExpectedCount);
	
			for (UDataTable* DT : ItemDataTableSources)
			{
				for (const TPair<FName, uint8*>& Pair : DT->GetRowMap())
				{
					FName  RowName = Pair.Key;
					FNAItemBaseTableRow* Row = DT->FindRow<FNAItemBaseTableRow>(RowName, TEXT("Mapping item meta data"));
					if (!Row || !Row->ItemClass) { continue; }
					FDataTableRowHandle Handle;
					Handle.DataTable = DT;
					Handle.RowName = RowName;
					ItemMetaDataMap.Emplace(Row->ItemClass.Get(), Handle);
#if	WITH_EDITOR
					ValidateItemRow(Row, RowName);
					SynchronizeItemCDOWithMeta(Row->ItemClass.Get(), Row, true);
#endif
				}
			}
		}
	}
	
	if (!ItemDataTableSources.IsEmpty() && !ItemMetaDataMap.IsEmpty()) {
		bMetaDataInitialized = true;
		UE_LOG(LogTemp, Warning, TEXT("[UNAItemEngineSubsystem::Initialize]  아이템 메타데이터 맵 초기화 완료"));
		
		// 글로벌 델리게이트에 바인딩
		FCoreUObjectDelegates::OnObjectPostCDOCompiled.AddUObject(this, &UNAItemEngineSubsystem::HandlePostCDOCompiled);
	}
}

#if	WITH_EDITOR
void UNAItemEngineSubsystem::CheckAndMappingItemClass(UClass* InItemActorClass)
{
	if (!InItemActorClass->IsChildOf<ANAItemActor>() || !InItemActorClass->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		return;
	}
	
	if (UClass* BPGC = ResolveToGeneratedItemClass(InItemActorClass))
	{
		InItemActorClass = BPGC;
	}
	
	if (!ItemMetaDataMap.Contains(InItemActorClass))
	{
		FNAItemBaseTableRow* CheckRow = nullptr;
		FDataTableRowHandle Handle;
		for (UDataTable* DT : ItemDataTableSources)
		{
			for (const TPair<FName, uint8*>& Pair : DT->GetRowMap())
			{
				FName  RowName = Pair.Key;
				FNAItemBaseTableRow* Row = DT->FindRow<FNAItemBaseTableRow>(RowName, TEXT("Search item meta data"));
				if (Row->ItemClass.Get() == InItemActorClass)
				{
					CheckRow = Row;
					Handle.DataTable = DT;
					Handle.RowName = RowName;
					ItemMetaDataMap.Emplace(Row->ItemClass, Handle);
					return;
				}
			}
		}

		if (!CheckRow)
		{
			for (UDataTable* DT : ItemDataTableSources)
			{
				if (DT->GetRowStruct() == FNAItemBaseTableRow::StaticStruct())
				{
					FNAItemBaseTableRow NewRow = FNAItemBaseTableRow(InItemActorClass);
					DT->AddRow(InItemActorClass->GetFName(), NewRow);
				}
			}
		}
	}
}

EItemMetaDirtyFlags UNAItemEngineSubsystem::FindChangedItemMetaFlags(bool bCheckMetaMap, const FDataTableRowHandle& RowHandle, const UObject* InCDO) const
{
	EItemMetaDirtyFlags Result = EItemMetaDirtyFlags::MF_None;

	if (!RowHandle.IsNull() && InCDO)
	{
		if (const FNAItemBaseTableRow* Row = RowHandle.GetRow<FNAItemBaseTableRow>(RowHandle.RowName.ToString()))
		{
			return FindChangedItemMetaFlags(bCheckMetaMap, Row, InCDO);
		}
	}
	return Result;
}

EItemMetaDirtyFlags UNAItemEngineSubsystem::FindChangedItemMetaFlags(bool bCheckMetaMap, const FNAItemBaseTableRow* RowData, const UObject* InCDO) const
{
	EItemMetaDirtyFlags Result = EItemMetaDirtyFlags::MF_None;

	if (!RowData || !InCDO)
	{
		ensure(false);
		return Result;
	}

	if (bCheckMetaMap && !ContainsItemMetaDataEntry(RowData))
	{
		ensureAlwaysMsgf(false,
			TEXT("[UNAItemEngineSubsystem::FindChangedItemMetaFlags]  메타데이터에 없는 데이터라는디")
			, *InCDO->GetName());
		return Result;
	}
	
	if (!InCDO->GetClass()->IsChildOf<ANAItemActor>() || !InCDO->HasAnyFlags(RF_ClassDefaultObject))
	{
		ensureAlwaysMsgf(false,
			TEXT("[UNAItemEngineSubsystem::FindChangedItemMetaFlags]  InCDO(%s)가 ANAItemActor 계열의 CDO가 아니었음")
			, *InCDO->GetName());
		return Result;
	}

	if (RowData->ItemClass.Get() != InCDO->GetClass())
	{
		ensureAlwaysMsgf(false,
			TEXT("[UNAItemEngineSubsystem::FindChangedItemMetaFlags]  RowData->ItemClass(%s), InCDO(%s) 둘이 다른 클래스라는데여")
			, *RowData->ItemClass.Get()->GetName(), *InCDO->GetClass()->GetName());
		return Result;
	}
	
	const ANAItemActor* ItemActorCDO = Cast<ANAItemActor>(InCDO);
	if (ItemActorCDO)
	{
		if (RowData->RootShapeType != ItemActorCDO->ItemRootShapeType
			|| !IsValid(ItemActorCDO->ItemRootShape))
		{
			EnumAddFlags(Result, EItemMetaDirtyFlags::MF_RootShape);
		}
		
		if (RowData->MeshType != ItemActorCDO->ItemMeshType
			|| !IsValid(ItemActorCDO->ItemMesh))
		{
			EnumAddFlags(Result, EItemMetaDirtyFlags::MF_Mesh);
		}
		else
		{
			if (USkeletalMeshComponent* SkeletalMeshComp = Cast<USkeletalMeshComponent>(ItemActorCDO->ItemMesh))
			{
				if (RowData->SkeletalMeshAssetData.SkeletalMesh != SkeletalMeshComp->GetSkeletalMeshAsset())
				{
					EnumAddFlags(Result, EItemMetaDirtyFlags::MF_MeshAsset);
				}
				if (RowData->SkeletalMeshAssetData.AnimClass != SkeletalMeshComp->GetAnimClass())
				{
					EnumAddFlags(Result, EItemMetaDirtyFlags::MF_MeshAnim);
				}
			}
			else if (UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(ItemActorCDO->ItemMesh))
			{
				if (RowData->StaticMeshAssetData.StaticMesh != StaticMeshComp->GetStaticMesh())
				{
					EnumAddFlags(Result, EItemMetaDirtyFlags::MF_MeshAsset);
				}
			}
		}
		
		if (!IsValid(ItemActorCDO->ItemInteractionButton))
		{
			EnumAddFlags(Result, EItemMetaDirtyFlags::MF_IxButton);
		}
		if (!IsValid(ItemActorCDO->ItemInteractionButtonText))
		{
			EnumAddFlags(Result, EItemMetaDirtyFlags::MF_IxButtonText);
		}
	}
	
	return Result;
}

void UNAItemEngineSubsystem::AddItemActorObj(ANAItemActor* InItemActorClass)
{
	
}

bool UNAItemEngineSubsystem::ContainsItemMetaDataHandle(const FDataTableRowHandle& RowHandle) const
{
	if (!RowHandle.IsNull())
	{
		if (const FNAItemBaseTableRow* Row = RowHandle.GetRow<FNAItemBaseTableRow>(RowHandle.RowName.ToString()))
		{
			if (UClass* ItemClass = Row->ItemClass)
			{
				return ContainsItemMetaClass(ItemClass);
			}
		}
	}
	return false;
}

bool UNAItemEngineSubsystem::ContainsItemMetaDataEntry(const FNAItemBaseTableRow* RowData) const
{
	if (RowData)
	{
		if (UClass* ItemClass = RowData->ItemClass.Get())
		{
			return ContainsItemMetaClass(ItemClass);
		}
	}
	return false;
}

bool UNAItemEngineSubsystem::ContainsItemMetaClass(UClass* InItemActorClass) const
{
	if (InItemActorClass->IsChildOf<ANAItemActor>())
	{
		return ItemMetaDataMap.Contains(InItemActorClass);
	}
	return false;
}

void UNAItemEngineSubsystem::HandlePostCDOCompiled(UObject* InCDO, const FObjectPostCDOCompiledContext& CompiledContext)
{
	// if (CompiledContext.bIsSkeletonOnly)
	// {
	// 	return;
	// }
	
	// ANAItemActor 파생인지, CDO인지, 블루프린트에서 파생된 클래스인지
	if (!InCDO->HasAnyFlags(RF_ClassDefaultObject) ||
		!InCDO->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		return;
	}
	if (ANAItemActor* ItemActorCDO = Cast<ANAItemActor>(InCDO))
	{
		// 1) 블루프린트 GeneratedClass와 에셋 가져오기
		UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(InCDO->GetClass());
		UBlueprint* BPAsset = BPGC ? Cast<UBlueprint>(BPGC->ClassGeneratedBy) : nullptr;
		if (!BPAsset) { return; }

		FNAItemBaseTableRow* ItemData = AccessItemMetaData(InCDO->GetClass());
		if (ItemData)
		{
			ItemData->CachedTransforms.MeshTransform = ItemActorCDO->ItemMesh->GetRelativeTransform();
			ItemData->CachedTransforms.ButtonTransform = ItemActorCDO->ItemInteractionButton->GetRelativeTransform();
			ItemData->CachedTransforms.ButtonTextTransform = ItemActorCDO->ItemInteractionButtonText->GetRelativeTransform();
			
			if (USphereComponent* RootSphere = Cast<USphereComponent>(ItemActorCDO->ItemRootShape))
			{
				ItemData->CachedTransforms.RootSphereRadius = RootSphere->GetScaledSphereRadius();
			}
			else if (UBoxComponent* RootBox = Cast<UBoxComponent>(ItemActorCDO->ItemRootShape))
			{
				ItemData->CachedTransforms.RootBoxExtent = RootBox->GetScaledBoxExtent();
			}
			else if (UCapsuleComponent* RootCapsule = Cast<UCapsuleComponent>(ItemActorCDO->ItemRootShape))
			{
				ItemData->CachedTransforms.RootCapsuleSize.X = RootCapsule->GetScaledCapsuleRadius();
				ItemData->CachedTransforms.RootCapsuleSize.X = RootCapsule->GetScaledCapsuleHalfHeight();
			}
			
			if (!CompiledContext.bIsSkeletonOnly)
			{
				SynchronizeItemCDOWithMeta(InCDO->GetClass(), ItemData, false);
			}
		}
	}
}

void UNAItemEngineSubsystem:: SynchronizeItemCDOWithMeta(UClass* InItemActorClass, const FNAItemBaseTableRow* RowData , bool bShouldRecompile) const
{
	if (!InItemActorClass || !RowData) { return;}
	
	ensureAlwaysMsgf(ContainsItemMetaClass(InItemActorClass),
		TEXT("[UNAItemEngineSubsystem::SynchronizeItemCDOWithMeta]  ItemMetaDataMap에 등록되지 않은 ItemClass."));
	
	ensureAlwaysMsgf(InItemActorClass == RowData->ItemClass.Get(),
			TEXT("[UNAItemEngineSubsystem::SynchronizeItemCDOWithMeta]  왜 둘이 다름 ???"));
	
	const bool bEnsureCheck = InItemActorClass->IsChildOf<ANAItemActor>() && InItemActorClass->HasAnyClassFlags(CLASS_CompiledFromBlueprint);
	ensureAlwaysMsgf(bEnsureCheck,
		TEXT("[UNAItemEngineSubsystem::SynchronizeItemCDOWithMeta]  ANAItemActor 파생이 아닌 블루프린트 클래스였음."));
	
	TArray<UClass*> Classes;
	
	UClass* BPClass = ResolveToGeneratedItemClass(InItemActorClass);
	BPClass = BPClass ? BPClass : InItemActorClass;
	Classes.Emplace(BPClass);
	
	//UClass* SkelClass = ResolveToSkeletalItemClass(InItemActorClass);
	//SkelClass = SkelClass ? SkelClass : InItemActorClass;
	//Classes.Emplace(SkelClass);

	if (Classes.IsEmpty())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[UNAItemEngineSubsystem::SynchronizeItemCDOWithMeta]  뭐야 뭔데 BP 클래스 왜 없는건데"));
		return;
	}
	
	EItemMetaDirtyFlags MetaDirtyFlags
		= FindChangedItemMetaFlags(IsItemMetaDataInitialized(), RowData, InItemActorClass->GetDefaultObject(false));
	
	if (MetaDirtyFlags == EItemMetaDirtyFlags::MF_None)
	{
		UE_LOG(LogTemp, Warning,
		TEXT("[UNAItemEngineSubsystem::SynchronizeItemCDOWithMeta]  메타데이터 업데이트 플래그 없음."));
		return;
	}
	
	if (UBlueprint* BPAsset = Cast<UBlueprint>(BPClass->ClassGeneratedBy))
	{
		BPAsset->Modify();
		
		const EObjectFlags MetaSubobjFlags = RF_Transient | RF_Public | RF_ArchetypeObject | RF_Transactional | RF_DefaultSubObject;
		
		for (UClass* ItemClass : Classes)
		{
			UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(ItemClass);
			ANAItemActor* ItemActorBPCDO = Cast<ANAItemActor>(ItemClass->GetDefaultObject(false));
			if (!IsValid(ItemClass) || !IsValid(BPGC) || !IsValid(ItemActorBPCDO))
			{
				ensure(false);
				continue;
			}
			
			ItemActorBPCDO->Modify();
			TArray<UActorComponent*> OldComponents;

			// Create new subobjects

			//USCS_Node* NewRootShapeNode = nullptr;
			if (EnumHasAnyFlags(MetaDirtyFlags, EItemMetaDirtyFlags::MF_RootShape))
			{
				if (ItemActorBPCDO->ItemRootShape)
				{
					OldComponents.Emplace(ItemActorBPCDO->ItemRootShape.Get());
				}

				switch (RowData->RootShapeType)
				{
				case EItemRootShapeType::IRT_Sphere:
					//NewRootShapeNode = SCS->CreateNode(USphereComponent::StaticClass(), TEXT("ItemRootShape(Sphere)"));
					ItemActorBPCDO->ItemRootShape = NewObject<USphereComponent>(
						ItemActorBPCDO, USphereComponent::StaticClass(),TEXT("ItemRootShape(Sphere)"), MetaSubobjFlags);
					break;
				case EItemRootShapeType::IRT_Box:
					//NewRootShapeNode = SCS->CreateNode(UBoxComponent::StaticClass(), TEXT("ItemRootShape(Box)"));
						ItemActorBPCDO->ItemRootShape = NewObject<UBoxComponent>(
							ItemActorBPCDO, UBoxComponent::StaticClass(),TEXT("ItemRootShape(Box)"), MetaSubobjFlags);
					break;
				case EItemRootShapeType::IRT_Capsule:
					//NewRootShapeNode = SCS->CreateNode(UCapsuleComponent::StaticClass(), TEXT("ItemRootShape(Capsule)"));
					ItemActorBPCDO->ItemRootShape = NewObject<UCapsuleComponent>(
						ItemActorBPCDO, UCapsuleComponent::StaticClass(),TEXT("ItemRootShape(Capsule)"),
						MetaSubobjFlags);
					break;

				default:
					ensureAlwaysMsgf(false,
						TEXT("[UNAItemEngineSubsystem::SynchronizeItemCDOWithMeta]  RowData->RootShapeType이 none이었음. ???" ));
					continue;
				}
				if (/*NewRootShapeNode == nullptr || */ItemActorBPCDO->ItemRootShape == nullptr)
				{
					ensureAlwaysMsgf(false,
					                 TEXT("[UNAItemEngineSubsystem::SynchronizeItemCDOWithMeta]  ItemRootShape 생성 실패. ???"));
					continue;
				}
				ItemActorBPCDO->ItemRootShapeType = RowData->RootShapeType;
				ItemActorBPCDO->AddOwnedComponent(ItemActorBPCDO->ItemRootShape);
				//BPGC->ComponentTemplates.AddUnique(ItemActorBPCDO->ItemRootShape);
				//SCS->AddNode(NewRootShapeNode);
			}

			//USCS_Node* NewMeshNode = nullptr;
			if (EnumHasAnyFlags(MetaDirtyFlags, EItemMetaDirtyFlags::MF_Mesh))
			{
				if (ItemActorBPCDO->ItemMesh)
				{
					OldComponents.Emplace(ItemActorBPCDO->ItemMesh.Get());
				}

				switch (RowData->MeshType)
				{
				case EItemMeshType::IMT_Static:
					//NewMeshNode = SCS->CreateNode(UStaticMeshComponent::StaticClass(), TEXT("ItemMesh(Static)"));
					ItemActorBPCDO->ItemMesh = NewObject<UStaticMeshComponent>(
						ItemActorBPCDO, UStaticMeshComponent::StaticClass(),TEXT("ItemMesh(Static)"), MetaSubobjFlags);
					break;
				case EItemMeshType::IMT_Skeletal:
					//NewMeshNode = SCS->CreateNode(USkeletalMeshComponent::StaticClass(), TEXT("ItemMesh(Skeletal)"));
					ItemActorBPCDO->ItemMesh = NewObject<USkeletalMeshComponent>(
						ItemActorBPCDO, USkeletalMeshComponent::StaticClass(),TEXT("ItemMesh(Skeletal)"),
						MetaSubobjFlags);
					break;

				default:
					ensureAlwaysMsgf(false,
					                 TEXT( "[UNAItemEngineSubsystem::SynchronizeItemCDOWithMeta]  RowData->MeshType이 None이었음. ???"));
					continue;
				}
				if (/*NewMeshNode == nullptr || */ItemActorBPCDO->ItemMesh == nullptr)
				{
					ensureAlwaysMsgf(false,
					                 TEXT("[UNAItemEngineSubsystem::SynchronizeItemCDOWithMeta]  ItemMesh 생성 실패. ???"));
					continue;
				}
				ItemActorBPCDO->ItemMeshType = RowData->MeshType;
				ItemActorBPCDO->AddOwnedComponent(ItemActorBPCDO->ItemMesh);
				//BPGC->ComponentTemplates.AddUnique(ItemActorBPCDO->ItemMesh);
				//SCS->AddNode(NewMeshNode);
				// if (NewRootShapeNode)
				// {
				// 	NewMeshNode->SetParent(NewRootShapeNode);
				// }
			}

			//USCS_Node* NewIxButtonNode = nullptr;
			if (EnumHasAnyFlags(MetaDirtyFlags, EItemMetaDirtyFlags::MF_IxButton))
			{
				if (ItemActorBPCDO->ItemInteractionButton)
				{
					OldComponents.Emplace(ItemActorBPCDO->ItemInteractionButton.Get());
				}

				//NewIxButtonNode = SCS->CreateNode(UBillboardComponent::StaticClass(), TEXT("ItemMesh(Skeletal)"));
				ItemActorBPCDO->ItemInteractionButton = NewObject<UBillboardComponent>(
					ItemActorBPCDO, UBillboardComponent::StaticClass(),TEXT("ItemInteractionButton"), MetaSubobjFlags);

				if (/*NewIxButtonNode == nullptr || */ItemActorBPCDO->ItemInteractionButton == nullptr)
				{
					ensureAlwaysMsgf(false,
					                 TEXT("[UNAItemEngineSubsystem::SynchronizeItemCDOWithMeta]  ItemInteractionButton 생성 실패. ???"));
					continue;
				}
				ItemActorBPCDO->AddOwnedComponent(ItemActorBPCDO->ItemInteractionButton);
				//BPGC->ComponentTemplates.AddUnique(ItemActorBPCDO->ItemInteractionButton);
				// SCS->AddNode(NewIxButtonNode);
				// if (NewMeshNode)
				// {
				// 	NewIxButtonNode->SetParent(NewMeshNode);
				// }
			}
			
			//USCS_Node* NewIxButtonTextNode = nullptr;
			if (EnumHasAnyFlags(MetaDirtyFlags, EItemMetaDirtyFlags::MF_IxButton))
			{
				if (ItemActorBPCDO->ItemInteractionButtonText)
				{
					OldComponents.Emplace(ItemActorBPCDO->ItemInteractionButtonText.Get());
				}

				//NewIxButtonTextNode = SCS->CreateNode(UTextRenderComponent::StaticClass(), TEXT("ItemMesh(Skeletal)"));
				ItemActorBPCDO->ItemInteractionButtonText = NewObject<UTextRenderComponent>(
					ItemActorBPCDO, UTextRenderComponent::StaticClass(),TEXT("ItemInteractionButtonText"),
					MetaSubobjFlags);

				if (/*NewIxButtonTextNode == nullptr || */ItemActorBPCDO->ItemInteractionButtonText == nullptr)
				{
					ensureAlwaysMsgf(false,
					                 TEXT("[UNAItemEngineSubsystem::SynchronizeItemCDOWithMeta]  ItemInteractionButtonText 생성 실패. ???"));
					continue;
				}
				ItemActorBPCDO->AddOwnedComponent(ItemActorBPCDO->ItemInteractionButtonText);
				//BPGC->ComponentTemplates.AddUnique(ItemActorBPCDO->ItemInteractionButtonText);
				//SCS->AddNode(NewIxButtonTextNode);
				// if (NewIxButtonNode)
				// {
				// 	NewIxButtonTextNode->SetParent(NewIxButtonNode);
				// }
			}

			// Hierarchy & Property settings

			ItemActorBPCDO->SetRootComponent(ItemActorBPCDO->ItemRootShape);
			ItemActorBPCDO->ItemRootShape->Mobility = EComponentMobility::Movable;

			ItemActorBPCDO->ItemMesh->SetupAttachment(ItemActorBPCDO->ItemRootShape);
			ItemActorBPCDO->ItemMesh->Mobility = EComponentMobility::Movable;
			ItemActorBPCDO->ItemMesh->SetRelativeTransform(RowData->CachedTransforms.MeshTransform);
			if (UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(ItemActorBPCDO->ItemMesh))
			{
				if (EnumHasAnyFlags(MetaDirtyFlags, EItemMetaDirtyFlags::MF_Mesh | EItemMetaDirtyFlags::MF_MeshAsset))
				{
					StaticMeshComp->SetStaticMesh(RowData->StaticMeshAssetData.StaticMesh);
				}

				ItemActorBPCDO->ItemFractureCollection = RowData->StaticMeshAssetData.FractureCollection;
				ItemActorBPCDO->ItemFractureCache = RowData->StaticMeshAssetData.FractureCache;
			}
			else if (USkeletalMeshComponent* SkeletalMeshComp = Cast<USkeletalMeshComponent>(ItemActorBPCDO->ItemMesh))
			{
				if (EnumHasAnyFlags(MetaDirtyFlags, EItemMetaDirtyFlags::MF_Mesh))
				{
					SkeletalMeshComp->SetSkeletalMesh(RowData->SkeletalMeshAssetData.SkeletalMesh);
					SkeletalMeshComp->SetAnimClass(RowData->SkeletalMeshAssetData.AnimClass.Get());
				}
				else if (EnumHasAnyFlags(MetaDirtyFlags, EItemMetaDirtyFlags::MF_MeshAsset))
				{
					SkeletalMeshComp->SetSkeletalMesh(RowData->SkeletalMeshAssetData.SkeletalMesh);
				}
				else if (EnumHasAnyFlags(MetaDirtyFlags, EItemMetaDirtyFlags::MF_MeshAnim))
				{
					SkeletalMeshComp->SetAnimClass(RowData->SkeletalMeshAssetData.AnimClass.Get());
				}
			}

			ItemActorBPCDO->ItemInteractionButton->SetupAttachment(ItemActorBPCDO->ItemMesh);
			ItemActorBPCDO->ItemInteractionButton->SetSprite(RowData->IconAssetData.Icon);

			ItemActorBPCDO->ItemInteractionButtonText->SetupAttachment(ItemActorBPCDO->ItemInteractionButton);

			ItemActorBPCDO->ItemDataID = NAME_None;
			
			if (!OldComponents.IsEmpty())
			{
				if (USimpleConstructionScript* SCS = BPAsset->SimpleConstructionScript)
				{
					for (UActorComponent* OldComponent : OldComponents)
					{
						BPGC->ComponentTemplates.RemoveSingle(OldComponent);
						if (USCS_Node* OldNode = SCS->FindSCSNode(OldComponent->GetFName()))
						{
							SCS->RemoveNode(OldNode, false);
						}
						OldComponent->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors);
						OldComponent->DestroyComponent();
						OldComponent = nullptr;
					}
					OldComponents.Empty();
				}
			}
			
			UE_LOG(LogTemp, Warning,
				   TEXT("[UNAItemEngineSubsystem::SynchronizeItemCDOWithMeta]  %s: CDO 동적 초기화 성공\n %s"),
				   *ItemClass->GetName(), *LexToString(ItemActorBPCDO->ItemRootShape->GetFlags()));
		}

		if (bShouldRecompile)
		{
			FCompilerResultsLog Results;
			FKismetEditorUtilities::CompileBlueprint(
				BPAsset,
				EBlueprintCompileOptions::BatchCompile | EBlueprintCompileOptions::SkipGarbageCollection,
				&Results
			);
			//FBlueprintEditorUtils::ReconstructAllNodes(BPAsset);
		}
		ANAItemActor::MarkItemActorCDOSynchronized(InItemActorClass);
		OnItemActorCDOPatched.Broadcast(InItemActorClass, RowData, MetaDirtyFlags);
		
		BPAsset->MarkPackageDirty();
	}
}

// 찐 블프 클래스에서 SKEL 클래스 찾기
UClass* UNAItemEngineSubsystem::ResolveToSkeletalItemClass(UClass* InItemActorClass) const
{
	if (!InItemActorClass) { return nullptr;}
	
	const bool bCheck = InItemActorClass->IsChildOf<ANAItemActor>() && InItemActorClass->HasAnyClassFlags(CLASS_CompiledFromBlueprint);
	if (!bCheck)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[UNAItemEngineSubsystem::ResolveToSkeletalItemClass]  (%s): ANAItemActor 파생이 아닌 블루프린트 클래스였음.")
			, *InItemActorClass->GetName());
		return nullptr;
	}
	if (UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(InItemActorClass))
	{
		if (UBlueprint* BPAsset = Cast<UBlueprint>(BPGC->ClassGeneratedBy))
		{
			return BPAsset->SkeletonGeneratedClass;
		}
	}

	// SKEL 클래스 없음
	UE_LOG(LogTemp, Warning,
		TEXT("[UNAItemEngineSubsystem::ResolveToSkeletalItemClass]  (%s): SKEL_ 클래스 못찾음.")
		, *InItemActorClass->GetName());
	return nullptr;
}

// SKEL 클래스에서 찐 블프 클래스 찾기
UClass* UNAItemEngineSubsystem::ResolveToGeneratedItemClass(UClass* InItemActorClass) const
{
	if (!InItemActorClass || InItemActorClass->IsChildOf<ANAItemActor>()
		||!InItemActorClass->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
	{
		return nullptr;
	}
	
	if (UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(InItemActorClass))
	{
		// 여기서 ClassGeneratedBy 는 이 클래스가 속한 UBlueprint 에셋 포인터
		if (UBlueprint* BPAsset = Cast<UBlueprint>(BPGC->ClassGeneratedBy))
		{
			// Blueprint->GeneratedClass 가 실제 런타임／PIE／게임에서 사용하는 BP\_…\_C 임
			if (BPAsset->GeneratedClass)
			{
				return BPAsset->GeneratedClass;
			}
		}
	}

	// 찐 블프 클래스 없음
	UE_LOG(LogTemp, Warning,
		TEXT("[UNAItemEngineSubsystem::ResolveToSkeletalItemClass]  (%s): 찐 블프 클래스 못찾음.")
		, *InItemActorClass->GetName());
	return nullptr;
}

void UNAItemEngineSubsystem::ValidateItemRow(const FNAItemBaseTableRow* RowData, const FName RowName) const
{
	const int32 Slot = RowData->NumericData.MaxSlotStackSize;
	const int32 Inv = RowData->NumericData.MaxInventoryStackSize;
	if (Inv != 0 && Slot > Inv)
	{
		ensureMsgf(
			false,
			TEXT("DataTable(%s): 오류! MaxSlotStackSize(%d) > MaxInventoryStackSize(%d)"),
			*RowName.ToString(), Slot, Inv);
	}
	else if (Inv != 0 && Slot == 0)
	{
		ensureMsgf(
			false,
			TEXT("DataTable(%s): 오류! MaxInventoryStackSize(%d)이 0보다 큰데, MaxSlotStackSize(%d)이 0 이었음"),
			*RowName.ToString(), Slot, Inv);
	}
}
#endif

void UNAItemEngineSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

UNAItemData* UNAItemEngineSubsystem::GetRuntimeItemData(const FName& InItemID) const
{
	UNAItemData* Value = nullptr;
	Value = RuntimeItemDataMap.Find(InItemID)->Get();
	return Value;
}

#if	WITH_EDITOR
FNAItemBaseTableRow* UNAItemEngineSubsystem::AccessItemMetaData(const TSubclassOf<ANAItemActor> InItemActorClass) const
{
	if (IsItemMetaDataInitialized())
	{
		UClass* sh1t = ResolveToGeneratedItemClass(InItemActorClass.Get());
		sh1t = sh1t ? sh1t : InItemActorClass.Get();
		if (sh1t)
		{
			if (const FDataTableRowHandle* Value = ItemMetaDataMap.Find(sh1t))
			{
				return Value->GetRow<FNAItemBaseTableRow>(Value->RowName.ToString());
			}
		}
	}
	
	return nullptr;
}

const FNAItemBaseTableRow* UNAItemEngineSubsystem::GetItemMetaData(UClass* InItemActorClass) const
{
	return AccessItemMetaData(InItemActorClass);
}
#endif

UNAItemData* UNAItemEngineSubsystem::CreateItemDataBySlot(UWorld* InWorld, const FNAInventorySlot& InInventorySlot)
{
	if (InInventorySlot.ItemMetaDataKey)
	{
		FDataTableRowHandle ItemMetaDTRowHandle = *(ItemMetaDataMap.Find(InInventorySlot.ItemMetaDataKey.Get()));
		if (ItemMetaDTRowHandle.IsNull()) {
			ensureAlwaysMsgf(false,
				TEXT("[UNAItemEngineSubsystem::CreateItemDataBySlot]  ItemMetaDataMap에 등록되지 않은 ItemActorClass임.  %s"),
				*InInventorySlot.ItemMetaDataKey.Get()->GetName());
			return nullptr;
		}
		
		UNAItemData* NewItemData = NewObject<UNAItemData>(this, NAME_None, RF_Transient);
		if (!NewItemData) {
			ensureAlwaysMsgf(false, TEXT("[UNAItemEngineSubsystem::CreateItemDataBySlot]  새로운 UNAItemData 객체 생성 실패"));
			return nullptr;
		}
		
		NewItemData->ItemMetaDataHandle = ItemMetaDTRowHandle;
		FString NameStr    = ItemMetaDTRowHandle.RowName.ToString();
		FString CountStr   = FString::FromInt(NewItemData->IDCount.GetValue());
		FString NewItemID  = NameStr + TEXT("_") + CountStr;
		
		NewItemData->ID = FName(*NewItemID);
		NewItemData->ItemState = static_cast<EItemState>(InInventorySlot.ItemState);

		// 3) 새로 생성한 UNAItemData 객체의 소유권을 런타임 때 아이템 데이터 추적용 Map으로 이관
		RuntimeItemDataMap.Emplace(NewItemData->ID, NewItemData);

		return RuntimeItemDataMap[NewItemData->ID];
	}

	ensureAlwaysMsgf(false, TEXT("[UNAItemEngineSubsystem::CreateItemDataBySlot]  InInventorySlot의 ItemMetaDataKey가 유효하지 않음."));
	return nullptr;
}
