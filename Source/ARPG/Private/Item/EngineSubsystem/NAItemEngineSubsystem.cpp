﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/EngineSubsystem/NAItemEngineSubsystem.h"

#include "Inventory/DataStructs/NAInventoryDataStructs.h"

#include "Item/ItemActor/NAItemActor.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/TextRenderComponent.h"

#include "Engine/SCS_Node.h"
#include "Abilities/GameplayAbility.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "Item/ItemActor/NAWeapon.h"
#include "Item/ItemDataStructs/NAWeaponDataStructs.h"
#include "Misc/ItemPatchHelper.h"

#if WITH_EDITOR
#include "FileHelpers.h"
#include "Kismet2/KismetEditorUtilities.h"
#endif

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
		
#if	WITH_EDITOR		
		// 글로벌 델리게이트에 바인딩
		FCoreUObjectDelegates::OnObjectPostCDOCompiled.AddUObject(this, &UNAItemEngineSubsystem::HandlePostCDOCompiled);
		// 에디터가 완전히 켜진 뒤에 한 번만 호출
		FCoreDelegates::OnPostEngineInit.AddUObject(this, &UNAItemEngineSubsystem::HandlePostEngineInit);
#endif		
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

	if ( const ANAWeapon* WeaponActorCDO = Cast<ANAWeapon>(InCDO) )
	{
		const FNAWeaponTableRow* WeaponRow = static_cast<const FNAWeaponTableRow*>( RowData );
		UNAMontageCombatComponent* CombatComponent = WeaponActorCDO->CombatComponent;
		
		if ( !CombatComponent ||
			 CombatComponent->GetMontage() != WeaponRow->FirearmStatistics.FirearmMontage ||
			 CombatComponent->GetAttackAbility() != WeaponRow->FirearmStatistics.GameplayAbility )
		{
			EnumAddFlags( Result, EItemMetaDirtyFlags::MF_Combat );
		}
	}
	
	
	return Result;
}

bool UNAItemEngineSubsystem::ContainsItemMetaDataHandle(const FDataTableRowHandle& RowHandle) const
{
	if (!RowHandle.IsNull())
	{
		if (const FNAItemBaseTableRow* Row = RowHandle.GetRow<FNAItemBaseTableRow>(RowHandle.RowName.ToString()))
		{
			if (UClass* ItemClass = Row->ItemClass.Get())
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

void UNAItemEngineSubsystem::HandlePostEngineInit() const
{
	// 한 번만 실행되도록 바인딩 해제
	FCoreDelegates::OnPostEngineInit.RemoveAll(this);

	// 더티된 모든 패키지를 저장
	FEditorFileUtils::SaveDirtyPackages(
		/*bPromptUserToSaveMap=*/    false,
		/*bPromptUserToSaveContent=*/true,
		/*bOnlyDirty=*/              true,
		/*bPromptForCheckout=*/      false
	);
}

void UNAItemEngineSubsystem::HandlePostCDOCompiled(UObject* InCDO, const FObjectPostCDOCompiledContext& CompiledContext)
{
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
			UPackage* DTPackage = ItemMetaDataMap[InCDO->GetClass()].DataTable->GetOutermost();
			DTPackage->Modify();

			if ( ItemActorCDO->ItemRootShape )
			{
				ItemData->CachedTransforms.RootTransform = ItemActorCDO->ItemRootShape->GetComponentTransform();	
			}
			if ( ItemActorCDO->ItemMesh )
			{
				ItemData->CachedTransforms.MeshTransform = ItemActorCDO->ItemMesh->GetRelativeTransform();	
			}
			if ( ItemActorCDO->ItemInteractionButton )
			{
				ItemData->CachedTransforms.ButtonTransform = ItemActorCDO->ItemInteractionButton->GetRelativeTransform();	
			}
			if ( ItemActorCDO->ItemInteractionButtonText )
			{
				ItemData->CachedTransforms.ButtonTextTransform = ItemActorCDO->ItemInteractionButtonText->GetRelativeTransform();	
			}
			
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
				ItemData->CachedTransforms.RootCapsuleSize.Y = RootCapsule->GetScaledCapsuleHalfHeight();
			}
			
			if (!CompiledContext.bIsSkeletonOnly)
			{
				SynchronizeItemCDOWithMeta(InCDO->GetClass(), ItemData, false);
			}
			
			DTPackage->MarkPackageDirty();

			TArray<UPackage*> PackagesToSave;
			PackagesToSave.Add(DTPackage);
			FEditorFileUtils::PromptForCheckoutAndSave(
				PackagesToSave,
				/*bCheckDirty=*/ false,    // 더티 여부 검사 안 함
				/*bPromptToSave=*/ false   // 사용자 프롬프트 없이 바로 저장
			);
		}
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

void UNAItemEngineSubsystem::SynchronizeItemCDOWithMeta(UClass* InItemActorClass, const FNAItemBaseTableRow* RowData , bool bShouldRecompile) const
{
	if (!InItemActorClass || !RowData) { return;}
	
	ensureAlwaysMsgf(ContainsItemMetaClass(InItemActorClass),
		TEXT("[UNAItemEngineSubsystem::SynchronizeItemCDOWithMeta]  ItemMetaDataMap에 등록되지 않은 ItemClass."));
	
	ensureAlwaysMsgf(InItemActorClass == RowData->ItemClass.Get(),
			TEXT("[UNAItemEngineSubsystem::SynchronizeItemCDOWithMeta]  왜 둘이 다름 ???"));
	
	const bool bEnsureCheck = InItemActorClass->IsChildOf<ANAItemActor>() && InItemActorClass->HasAnyClassFlags(CLASS_CompiledFromBlueprint);
	ensureAlwaysMsgf(bEnsureCheck,
		TEXT("[UNAItemEngineSubsystem::SynchronizeItemCDOWithMeta]  ANAItemActor 파생이 아닌 블루프린트 클래스였음."));

	const EItemMetaDirtyFlags MetaDirtyFlags
		= FindChangedItemMetaFlags(IsItemMetaDataInitialized(), RowData, InItemActorClass->GetDefaultObject(false));
	
	if (MetaDirtyFlags == EItemMetaDirtyFlags::MF_None)
	{
		UE_LOG(LogTemp, Warning,
		TEXT("[UNAItemEngineSubsystem::SynchronizeItemCDOWithMeta]  메타데이터 업데이트 플래그 없음."));
		return;
	}

	UClass* BPClass = ResolveToGeneratedItemClass(InItemActorClass);
	BPClass = BPClass ? BPClass : InItemActorClass;
	
	if (UBlueprint* BPAsset = Cast<UBlueprint>(BPClass->ClassGeneratedBy))
	{
		UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(BPClass);
		ANAItemActor* ItemActorBPCDO = Cast<ANAItemActor>(BPClass->GetDefaultObject(false));
		if (!IsValid(BPClass) || !IsValid(BPGC) || !IsValid(ItemActorBPCDO))
		{
			ensure(false);
			return;
		}
		
		BPAsset->Modify();
		ItemActorBPCDO->Modify();

		const EObjectFlags MetaSubobjFlags = ItemActorBPCDO->GetMaskedFlags(RF_PropagateToSubObjects) | RF_DefaultSubObject;

		TArray<UActorComponent*> OldComponents;

		// Create new subobjects
		FItemPatchHelper::UpdateDirtyComponent<
			decltype(ItemActorBPCDO->ItemRootShape)::ElementType,
			EItemMetaDirtyFlags::MF_RootShape,
			FRootShapeSpawnPredication,
			FRootShapeUpdatePredication>
		(
			MetaDirtyFlags,
			OldComponents,
			ItemActorBPCDO,
			&ItemActorBPCDO->ItemRootShape,
			RowData,
			TEXT( "" ),
			MetaSubobjFlags,
			false
		);

		FItemPatchHelper::UpdateDirtyComponent<
			decltype(ItemActorBPCDO->ItemMesh)::ElementType,
			EItemMetaDirtyFlags::MF_Mesh,
			FMeshSpawnPredication,
			FMeshUpdatePredication>
		(
			MetaDirtyFlags,
			OldComponents,
			ItemActorBPCDO,
			&ItemActorBPCDO->ItemMesh,
			RowData,
			TEXT( "" ),
			MetaSubobjFlags,
			false
		);

		FItemPatchHelper::UpdateDirtyComponent<
			decltype(ItemActorBPCDO->ItemInteractionButton)::ElementType,
			EItemMetaDirtyFlags::MF_IxButton,
			FItemPatchHelper::FDefaultSpawnPredication<decltype(ItemActorBPCDO->ItemInteractionButton)::ElementType>,
			FInteractionButtonUpdatePredication>
		(
			MetaDirtyFlags,
			OldComponents,
			ItemActorBPCDO,
			&ItemActorBPCDO->ItemInteractionButton,
			RowData,
			TEXT( "ItemInteractionButton" ),
			MetaSubobjFlags,
			false
		);

		FItemPatchHelper::UpdateDirtyComponent<
			decltype(ItemActorBPCDO->ItemInteractionButtonText)::ElementType,
			EItemMetaDirtyFlags::MF_IxButtonText,
			FItemPatchHelper::FDefaultSpawnPredication<decltype(ItemActorBPCDO->ItemInteractionButtonText)::ElementType>,
			FInteractionButtonTextUpdatePredication>
		(
			MetaDirtyFlags,
			OldComponents,
			ItemActorBPCDO,
			&ItemActorBPCDO->ItemInteractionButtonText,
			RowData,
			TEXT( "ItemInteractionButtonText" ),
			MetaSubobjFlags,
			false
		);

		if ( ANAWeapon* WeaponBPCDO = Cast<ANAWeapon>( ItemActorBPCDO ) )
		{
			FItemPatchHelper::UpdateDirtyComponent<
				UNAMontageCombatComponent,
				EItemMetaDirtyFlags::MF_Combat,
				FItemPatchHelper::FDefaultSpawnPredication<UNAMontageCombatComponent>,
				FCombatUpdatePredication>
			(
				MetaDirtyFlags,
				OldComponents,
				WeaponBPCDO,
				&WeaponBPCDO->CombatComponent,
				RowData,
				TEXT( "CombatComponent" ),
				MetaSubobjFlags,
				false
			);
		}

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
					OldComponent->ClearFlags( RF_Standalone | RF_Public );
					OldComponent->DestroyComponent(); // 안에서 MarkAsGarbage 처리됨
					CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS );
					OldComponent = nullptr;
				}
				OldComponents.Empty();
			}
		}

		UE_LOG(LogTemp, Warning,
		       TEXT("[UNAItemEngineSubsystem::SynchronizeItemCDOWithMeta]  %s: CDO 동적 초기화 성공\n %s"),
		       *BPClass->GetName(), *LexToString(ItemActorBPCDO->ItemRootShape->GetFlags()));

		if (bShouldRecompile)
		{
			FCompilerResultsLog Results;
			FKismetEditorUtilities::CompileBlueprint(
				BPAsset,
				EBlueprintCompileOptions::BatchCompile | EBlueprintCompileOptions::SkipGarbageCollection,
				&Results
			);
		}
		ANAItemActor::MarkItemActorCDOSynchronized(InItemActorClass);
		OnItemActorCDOPatched.Broadcast(InItemActorClass, RowData, MetaDirtyFlags);

		BPAsset->MarkPackageDirty();
	}
}

bool UNAItemEngineSubsystem::ContainsItemMetaClass(UClass* InItemActorClass) const
{
	if (InItemActorClass->IsChildOf<ANAItemActor>())
	{
		return ItemMetaDataMap.Contains(InItemActorClass);
	}
	return false;
}
#endif

void UNAItemEngineSubsystem::ValidateItemRow(const FNAItemBaseTableRow* RowData, const FName RowName) const
{
	const int32 Slot = RowData->NumericData.MaxSlotStackSize;
	const int32 Inv = RowData->NumericData.MaxInventoryHoldCount;
	if (Inv != 0 && Slot > Inv)
	{
		ensureMsgf(
			false,
			TEXT("DataTable(%s): 오류! MaxSlotStackSize(%d) > MaxInventoryHoldCount(%d)"),
			*RowName.ToString(), Slot, Inv);
	}
	else if (Inv != 0 && Slot == 0)
	{
		ensureMsgf(
			false,
			TEXT("DataTable(%s): 오류! MaxInventoryHoldCount(%d)이 0보다 큰데, MaxSlotStackSize(%d)이 0 이었음"),
			*RowName.ToString(), Slot, Inv);
	}
}

void UNAItemEngineSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

UNAItemData* UNAItemEngineSubsystem::CreateItemDataCopy(const UNAItemData* SourceItemData)
{
	if (!IsValid(SourceItemData))
	{
		ensureAlwaysMsgf(false, TEXT("[UNAItemEngineSubsystem::CreateItemDataCopy]  SourceItemData was null."));
		return nullptr;
	}

	if (SourceItemData->ItemMetaDataHandle.IsNull())
	{
		ensureAlwaysMsgf(false, TEXT("[UNAItemEngineSubsystem::CreateItemDataCopy]  SourceItemData's ItemMetaDataHandle was null."));
		return nullptr;
	}
	
	// 1) DuplicateObject로 원본을 복제합니다 (생성자 로직은 실행되지 않음).
	UNAItemData* Duplicated = DuplicateObject<UNAItemData>(SourceItemData, this);
	if (!Duplicated)
	{
		ensureAlwaysMsgf(false, TEXT("[UNAItemEngineSubsystem::CreateItemDataCopy]  Failed to duplicate item data."));
		return nullptr;
	}

	// 2) 생성자가 실행되지 않았으니, IDCount를 수동으로 증가
	//    FThreadSafeCounter::Increment()은 증가된 신규 값을 반환
	int32 NewNumber = UNAItemData::IDCount.Increment();
	Duplicated->IDNumber = NewNumber;

	// 3) ID를 “RowName + NewNumber” 형태로 다시 세팅
	FString NameStr;
	NameStr = Duplicated->ItemMetaDataHandle.RowName.ToString();
	FString CountStr = FString::FromInt(Duplicated->IDNumber);
	FString NewItemID = NameStr + TEXT("_") + CountStr;

	Duplicated->ID = FName(*NewItemID);

	// 4) 새로 생성한 UNAItemData 객체의 소유권을 런타임 때 아이템 데이터 추적용 Map으로 이관
	RuntimeItemDataMap.Emplace(Duplicated->ID, Duplicated);

	return RuntimeItemDataMap[Duplicated->ID].Get();
}

UNAItemData* UNAItemEngineSubsystem::GetRuntimeItemData(const FName& InItemID) const
{
	UNAItemData* Value = nullptr;
	if (!InItemID.IsNone())
	{
		Value = RuntimeItemDataMap.Find(InItemID)->Get();
	}
	return Value;
}

FNAItemBaseTableRow* UNAItemEngineSubsystem::AccessItemMetaData(UClass* InItemActorClass) const
{
	if (!InItemActorClass->IsChildOf<ANAItemActor>())
	{
		return nullptr;
	}
	if (IsItemMetaDataInitialized())
	{
		UClass* BPClass = nullptr;
#if WITH_EDITOR
		BPClass = ResolveToGeneratedItemClass(InItemActorClass);
#endif
		BPClass = BPClass ? BPClass : InItemActorClass;
		if (BPClass)
		{
			if (const FDataTableRowHandle* Value = ItemMetaDataMap.Find(BPClass))
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
