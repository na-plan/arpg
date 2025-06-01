#pragma once
#include "Item/ItemData/NAItemData.h"

struct FItemPatchHelper
{
	template <typename T>
	struct FDefaultSpawnPredication
	{
		virtual ~FDefaultSpawnPredication() = default;

		virtual T* operator()( UObject* InOuter, const FName& InComponentName, const EObjectFlags InObjectFlags, const FNAItemBaseTableRow* InRow ) const
		{
			return NewObject<T>( InOuter, T::StaticClass(), InComponentName, InObjectFlags );
		}
	};

	template <typename T>
	struct FDefaultUpdatePredication
	{
		virtual ~FDefaultUpdatePredication() = default;

		virtual void operator()( AActor* InOuter, T* InComponent, T* InOldComponent, const FNAItemBaseTableRow* InRow, const EItemMetaDirtyFlags DirtyFlags ) const
		{ }
	};

	template
	<typename ComponentT,
	EItemMetaDirtyFlags TargetFlag,
	typename SpawnPredication = FDefaultSpawnPredication<ComponentT>,
	typename PatchPredication = FDefaultUpdatePredication>
	static void UpdateDirtyComponent(
		const EItemMetaDirtyFlags MetaDirtyFlags,
		TArray<UActorComponent*>& OutOldComponents,
		AActor* InCDO,
		TObjectPtr<ComponentT>* InComponent,
		const FNAItemBaseTableRow* InRow,
		const FName& ComponentName,
		const EObjectFlags InObjectFlags,
		bool bRegister )
	{
		static SpawnPredication SpawnPredicate{};
		static PatchPredication PatchPredicate{};
	
		if ( EnumHasAnyFlags( MetaDirtyFlags, TargetFlag ) )
		{
			TObjectPtr<ComponentT> OldComponent = *InComponent;

			if ( OldComponent )
			{
				OutOldComponents.Emplace( OldComponent.Get() );
			}

			*InComponent = SpawnPredicate( InCDO, ComponentName, InObjectFlags, InRow );
			PatchPredicate( InCDO, (*InComponent).Get(), OldComponent.Get(), InRow, MetaDirtyFlags );
			InCDO->AddOwnedComponent( (*InComponent).Get() );
			if ( bRegister )
			{
				(*InComponent).Get()->CreationMethod = EComponentCreationMethod::Native;
				InCDO->AddInstanceComponent( (*InComponent).Get() );
				(*InComponent).Get()->RegisterComponent();
			}
		}
	}

	template
	<typename ComponentT,
	EItemMetaDirtyFlags TargetFlag,
	typename SpawnPredication = FDefaultSpawnPredication<ComponentT>,
	typename PatchPredication = FDefaultUpdatePredication>
	static void UpdateDirtyComponent(
		const EItemMetaDirtyFlags MetaDirtyFlags,
		TArray<UActorComponent*>& OutOldComponents,
		AActor* InCDO,
		ComponentT** InComponent,
		const FNAItemBaseTableRow* InRow,
		const FName& ComponentName,
		const EObjectFlags InObjectFlags,
		bool bRegister  )
	{
		static SpawnPredication SpawnPredicate{};
		static PatchPredication PatchPredicate{};
		
		if ( EnumHasAnyFlags(MetaDirtyFlags, TargetFlag) )
		{
			ComponentT* OldComponent = *InComponent;
			
			if ( OldComponent )
			{
				OutOldComponents.Emplace( OldComponent );
			}

			*InComponent = SpawnPredicate( InCDO, ComponentName, InObjectFlags, InRow );
			PatchPredicate( InCDO, *InComponent, OldComponent, InRow, MetaDirtyFlags );
			InCDO->AddOwnedComponent( *InComponent );
			if ( bRegister )
			{
				(*InComponent)->CreationMethod = EComponentCreationMethod::Native;
				InCDO->AddInstanceComponent( *InComponent );
				(*InComponent)->RegisterComponent();
			}
		}
	}
};
