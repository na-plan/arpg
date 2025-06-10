#pragma once

#include "GameFramework/Actor.h"
#include "Interaction/NAInteractableInterface.h"
#include "Item/NAItemUseInterface.h"
#include "Item/EngineSubsystem/NAItemEngineSubsystem.h"
#include "NAItemActor.generated.h"

class UTextRenderComponent;
class UNAMontageCombatComponent;
class UBillboardComponent;

UENUM()
enum class EItemSubobjDirtyFlags : uint8
{
	ISDF_None	= (0x0),
	
	ISDF_CollisionShape		= (1<<0),
	ISDF_MeshType				= (1<<1),
	//MF_IxButtonSprite	= (1<<4),
	//MF_IxButtonText		= (1<<5)
};
ENUM_CLASS_FLAGS(EItemSubobjDirtyFlags)

UCLASS(Abstract)
class ARPG_API ANAItemActor : public AActor, public INAInteractableInterface, public INAItemUseInterface
{
	GENERATED_BODY()

	friend class UNAItemEngineSubsystem;
public:
	ANAItemActor(const FObjectInitializer& ObjectInitializer);
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void PostLoad() override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Item Actor")
	UNAItemData* GetItemData() const;

	UFUNCTION(BlueprintCallable, Category = "Item Actor")
	bool HasValidItemID() const;

	static void TransferItemDataToDuplicatedActor(ANAItemActor* OldItemActor, ANAItemActor* NewDuplicated)
	{
		if ( UNAItemEngineSubsystem::Get() && OldItemActor && NewDuplicated)
		{
			if (UNAItemEngineSubsystem::Get()->DestroyRuntimeItemData(NewDuplicated->ItemDataID))
			{
				NewDuplicated->ItemDataID = OldItemActor->ItemDataID;
				if (OldItemActor->InteractableInterfaceRef && NewDuplicated->InteractableInterfaceRef)
				{
					INAInteractableInterface::TransferInteractableStateToDuplicatedActor(
						OldItemActor->InteractableInterfaceRef
						, NewDuplicated->InteractableInterfaceRef); // 어우 길어
				}

				OldItemActor->ItemDataID = NAME_None;
				OldItemActor->Destroy();
			}
		}
	}
	
protected:
	// OnItemDataInitialized: BP 확장 가능
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnItemDataInitialized();
	virtual void OnItemDataInitialized_Implementation();
	
	virtual EItemSubobjDirtyFlags CheckDirtySubobjectFlags(const FNAItemBaseTableRow* MetaData) const;

	UFUNCTION()
	void OnActorBeginOverlap_Impl( AActor* OverlappedActor, AActor* OtherActor );
	UFUNCTION()
	void OnActorEndOverlap_Impl( AActor* OverlappedActor, AActor* OtherActor );
	
private:
	void InitItemData();
	void VerifyInteractableData();

protected:
	UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadOnly, Category="Item Actor | Root Sphere")
	TObjectPtr<class USphereComponent> RootSphere;
	
	UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadOnly, Category="Item Actor | Collision Shape")
	TObjectPtr<UShapeComponent> ItemCollision;
	
	UPROPERTY(Instanced, VisibleAnywhere, Category = "Item Actor | Mesh")
	TObjectPtr<UMeshComponent> ItemMesh;

	UPROPERTY(VisibleAnywhere, Category = "Item Actor | Static Mesh")
	TObjectPtr<class UGeometryCollection> ItemFractureCollection;
	
	UPROPERTY(VisibleAnywhere, Category = "Item Actor | Static Mesh")
	TObjectPtr<class UGeometryCollectionCache> ItemFractureCache;

	UPROPERTY(VisibleAnywhere, Category = "Item Actor | Static Mesh")
	TObjectPtr<class UNAItemWidgetComponent> ItemWidgetComponent;

private:
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Item Actor", meta = (AllowPrivateAccess = "true"))
	FName ItemDataID;

	//======================================================================================================================
	// Interactable Interface Implements
	//======================================================================================================================
public:
	virtual bool CanInteract_Implementation() const override;
	//virtual bool CanInteract_Implementation() const override;
	virtual void NotifyInteractableFocusBegin_Implementation(AActor* InteractableActor, AActor* InteractorActor) override;
	virtual void NotifyInteractableFocusEnd_Implementation(AActor* InteractableActor, AActor* InteractorActor) override;

	virtual void BeginInteract_Implementation(AActor* Interactor) override;
	virtual void EndInteract_Implementation(AActor* Interactor) override;
	virtual bool ExecuteInteract_Implementation(AActor* Interactor) override;

	virtual bool IsOnInteract_Implementation() const override;
	
	virtual void DisableOverlapDuringInteraction() override;

protected:
	/** 자기 자신(this)이 구현한 인터페이스를 보관 */
	UPROPERTY()
	TScriptInterface<INAInteractableInterface> InteractableInterfaceRef = nullptr;
};

