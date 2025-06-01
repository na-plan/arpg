#pragma once

#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Interaction/NAInteractableInterface.h"
#include "Item/ItemData/NAItemData.h"
#include "Misc/ItemPatchHelper.h"
#include "NAItemActor.generated.h"

class UTextRenderComponent;
class UNAMontageCombatComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnItemActorInitialized, ANAItemActor*, InitializedItemActor );

struct FMeshUpdatePredication : FItemPatchHelper::FDefaultUpdatePredication<UMeshComponent>
{
	virtual void operator()( AActor* InOuter, UMeshComponent* InComponent, UMeshComponent* InOldComponent,
		const FNAItemBaseTableRow* InRow, const EItemMetaDirtyFlags DirtyFlags ) const override;
};

struct FMeshInstanceUpdatePredication : FMeshUpdatePredication
{
	virtual void operator()( AActor* InOuter, UMeshComponent* InComponent, UMeshComponent* InOldComponent,
		const FNAItemBaseTableRow* InRow, const EItemMetaDirtyFlags DirtyFlags ) const override;
};

struct FRootShapeUpdatePredication : FItemPatchHelper::FDefaultUpdatePredication<UShapeComponent>
{
	virtual void operator()( AActor* InOuter, UShapeComponent* InComponent, UShapeComponent* InOldComponent,
		const FNAItemBaseTableRow* InRow, const EItemMetaDirtyFlags DirtyFlags ) const override;
};

struct FRootShapeInstanceUpdatePredication : FRootShapeUpdatePredication
{
	virtual void operator()( AActor* InOuter, UShapeComponent* InComponent, UShapeComponent* InOldComponent,
		const FNAItemBaseTableRow* InRow, const EItemMetaDirtyFlags DirtyFlags ) const override;
};

struct FInteractionButtonUpdatePredication : FItemPatchHelper::FDefaultUpdatePredication<UBillboardComponent>
{
	virtual void operator()( AActor* InOuter, UBillboardComponent* InComponent, UBillboardComponent* InOldComponent,
		const FNAItemBaseTableRow* InRow, const EItemMetaDirtyFlags DirtyFlags ) const override;
};

struct FInteractionButtonInstanceUpdatePredication : FInteractionButtonUpdatePredication
{
	virtual void operator()( AActor* InOuter, UBillboardComponent* InComponent, UBillboardComponent* InOldComponent,
		const FNAItemBaseTableRow* InRow, const EItemMetaDirtyFlags DirtyFlags ) const override;
};

struct FInteractionButtonTextUpdatePredication : FItemPatchHelper::FDefaultUpdatePredication<UTextRenderComponent>
{
	virtual void operator()( AActor* InOuter, UTextRenderComponent* InComponent, UTextRenderComponent* InOldComponent,
		const FNAItemBaseTableRow* InRow, const EItemMetaDirtyFlags DirtyFlags ) const override;
};

struct FInteractionButtonTextInstanceUpdatePredication : FInteractionButtonTextUpdatePredication
{
	virtual void operator()( AActor* InOuter, UTextRenderComponent* InComponent, UTextRenderComponent* InOldComponent,
		const FNAItemBaseTableRow* InRow, const EItemMetaDirtyFlags DirtyFlags ) const override;
};

struct FRootShapeSpawnPredication : FItemPatchHelper::FDefaultSpawnPredication<UShapeComponent>
{
	virtual UShapeComponent* operator()( UObject* InOuter, const FName& InComponentName, const EObjectFlags InObjectFlags,
	                                     const FNAItemBaseTableRow* InRow ) const override;
};

struct FMeshSpawnPredication : FItemPatchHelper::FDefaultSpawnPredication<UMeshComponent>
{
	virtual UMeshComponent* operator()( UObject* InOuter, const FName& InComponentName, const EObjectFlags InObjectFlags,
	                                    const FNAItemBaseTableRow* InRow ) const override;
};

UCLASS(Abstract)
class ARPG_API ANAItemActor : public AActor, public INAInteractableInterface
{
	GENERATED_BODY()

	friend class UNAItemEngineSubsystem;
	friend struct FRootShapeUpdatePredication;
	friend struct FMeshUpdatePredication;
	friend struct FInteractionButtonUpdatePredication;
	friend struct FInteractionButtonTextUpdatePredication;
public:
	ANAItemActor(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
	virtual void PostLoadSubobjects( FObjectInstancingGraph* OuterInstanceGraph ) override;
	virtual void PostActorCreated() override;
	virtual void PreDuplicate(FObjectDuplicationParameters& DupParams) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
#if WITH_EDITOR
	// 블루프린트 에디터에서 프로퍼티 바뀔 때
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
protected:
	virtual void ExecuteItemPatch(UClass* ClassToPatch, const FNAItemBaseTableRow* PatchData, EItemMetaDirtyFlags PatchFlags);
#endif
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Item Actor")
	const UNAItemData* GetItemData() const;
	
protected:
	// OnItemDataInitialized: BP 확장 가능
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnItemDataInitialized();
	virtual void OnItemDataInitialized_Implementation();

	// 파생 클래스 오버라이딩 전용
	// @parm	InDataTableRowHandle: ANAItemActor::InitItemActor_Internal에서 유효성 검사 실행한 뒤 전달됨
	virtual void InitItemActor_Impl();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnItemActorInitialized();
	virtual void OnItemActorInitialized_Implementation();

private:
	void BeginItemInitialize_Internal();
	void InitItemData_Internal();
	void InitItemActor_Internal();
	void VerifyInteractableData_Internal();
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Item Actor")
	FOnItemActorInitialized OnItemActorInitializedDelegate;
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Item Actor | Root Shape")
	EItemRootShapeType ItemRootShapeType = EItemRootShapeType::IRT_None;
	UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadOnly, Category="Item Actor | Root Shape")
	TObjectPtr<UShapeComponent> ItemRootShape;

	UPROPERTY(BlueprintReadOnly, Category="Item Actor | Mesh")
	EItemMeshType ItemMeshType = EItemMeshType::IMT_None;
	UPROPERTY(Instanced, VisibleAnywhere, Category = "Item Actor | Mesh")
	TObjectPtr<UMeshComponent> ItemMesh;

	UPROPERTY(VisibleAnywhere, Category = "Item Actor | Static Mesh")
	TObjectPtr<class UGeometryCollection> ItemFractureCollection;
	UPROPERTY(VisibleAnywhere, Category = "Item Actor | Static Mesh")
	TObjectPtr<class UGeometryCollectionCache> ItemFractureCache;

	UPROPERTY(Instanced, VisibleAnywhere, Category="Item Actor | Interaction Button")
	TObjectPtr<UBillboardComponent> ItemInteractionButton;

	UPROPERTY(Instanced, VisibleAnywhere, Category="Item Actor | Interaction Button")
	TObjectPtr<class UTextRenderComponent> ItemInteractionButtonText;

private:
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Item Actor", meta = (AllowPrivateAccess = "true"))
	FName ItemDataID;

	uint8 bItemDataInitialized :1;
	
	struct NAItemActorProp
	{
		enum class Flag : uint16
		{
			None						= 0,
			
			NeedsUpdate					= 1 << 0,
		
			ItemRootShapeType			= 1 << 1,
			ItemRootShape				= 1 << 2,
			ItemMeshType				= 1 << 3,
			ItemMesh					= 1 << 4,
			ItemFractureCollection		= 1 << 5,
			ItemFractureCache			= 1 << 6,
			ItemAnimClass				= 1 << 7,
			ItemInteractionButton		= 1 << 8,
			ItemInteractionButtonText	= 1 << 9,
			ItemDataID					= 1 << 10,
		};

		static constexpr Flag AllProps =
			static_cast<Flag>(
				// (1 << (11 + 1)) - (1 << 0)
				(static_cast<uint16>(Flag::ItemDataID) << 1)
				- static_cast<uint16>(Flag::NeedsUpdate)
			);

		static constexpr bool HasAnyFlags(const Flag& Value, const Flag Test)
		{
			return (static_cast<uint16>(Value) & static_cast<uint16>(Test)) != 0;
		}
		
		static constexpr bool HasAllFlags(const Flag& Value, const Flag Test)
		{
			return (static_cast<uint16>(Value) & static_cast<uint16>(Test))
				 == static_cast<uint16>(Test);
		}
		
		static constexpr void AddFlags(Flag& Value, Flag ToAdd)
		{
			uint16 NewBits = static_cast<uint16>(Value) | static_cast<uint16>(ToAdd);
						
			Value = static_cast<Flag>(NewBits);
		}
		
		static constexpr void RemoveFlags(Flag& Value, Flag ToRemove)
		{
			Value = static_cast<Flag>(static_cast<uint16>(Value) & ~static_cast<uint16>(ToRemove));
		}
	};
	NAItemActorProp::Flag PropertyFlags;

	static void MarkItemActorCDOSynchronized(TSubclassOf<ANAItemActor> ItemActorClass)
	{
		if (ItemActorClass)
		{
			if (ANAItemActor* CDO = Cast<ANAItemActor>(ItemActorClass.Get()->GetDefaultObject(false)))
			{
				CDO->bCDOSynchronizedWithMeta = true;
			}
		}
	}
	static bool IsItemActorCDOSynchronizedWithMeta(TSubclassOf<ANAItemActor> ItemActorClass)
	{
		if (ItemActorClass)
		{
			if (ANAItemActor* CDO = Cast<ANAItemActor>(ItemActorClass.Get()->GetDefaultObject(false)))
			{
				return CDO->bCDOSynchronizedWithMeta;
			}
		}
		return false;
	}
	// 이 변수 건들면 안됨
	UPROPERTY(Transient)
	uint8 bCDOSynchronizedWithMeta : 1 = false;

//======================================================================================================================
// Interactable Interface Implements
//======================================================================================================================
public:
	virtual FNAInteractableData GetInteractableData_Implementation() const override;
	virtual const FNAInteractableData& GetInteractableData_Internal() const override;
	virtual void SetInteractableData_Implementation(const FNAInteractableData& NewInteractableData) override;
	virtual bool CanUseRootAsTriggerShape_Implementation() const override;
	virtual bool CanInteract_Implementation() const override;
	virtual void NotifyInteractableFocusBegin_Implementation(AActor* InteractableActor, AActor* InteractorActor) override;
	virtual void NotifyInteractableFocusEnd_Implementation(AActor* InteractableActor, AActor* InteractorActor) override;

	virtual void BeginInteract_Implementation(AActor* Interactor) override;
	virtual void EndInteract_Implementation(AActor* Interactor) override;
	virtual void ExecuteInteract_Implementation(AActor* Interactor) override;

	virtual bool IsOnInteract_Implementation() const override;

protected:
	/** 자기 자신(this)이 구현한 인터페이스를 보관 */
	UPROPERTY()
	TScriptInterface<INAInteractableInterface> InteractableInterfaceRef = nullptr;
	
	// INAInteractableInterface 메서드 x, ANAItemActor의 파생 클래스에서 구현해야하는 매크로 훅 함수
	// 오버라이딩이 필요하다면, 파생 클래스 선언 부 안에 IMPLEMENT_MACROHOOK_GetInitInteractableDataParams 매크로 쓰셈
	virtual void GetInitInteractableDataParams_MacroHook(
		ENAInteractableType& OutInteractableType,
		FString&			 OutInteractableName,
		/*FString&			 OutInteractableScript,*/
		float&				 OutInteractableDuration,
		int32&				 OutQuantity) const;
	
private:
	virtual void SetInteractableDataToBaseline_Implementation(
		ENAInteractableType InInteractableType,
		const FString&				InInteractionName,
		/*const FString&				InInteractionScript,*/
		float				InInteractionDuration,
		int32				InQuantity) override final;
};