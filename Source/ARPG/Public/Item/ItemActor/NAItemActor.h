#pragma once

#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Interaction/NAInteractableInterface.h"
#include "NAItemActor.generated.h"

class UTextRenderComponent;
class UNAMontageCombatComponent;
class UBillboardComponent;

UENUM()
enum class EItemSubobjDirtyFlags : uint8
{
	MF_None	= (0x0),
	
	MF_RootShape		= (1<<0),
	MF_Mesh				= (1<<1),
	MF_IxButtonSprite	= (1<<4),
	MF_IxButtonText		= (1<<5),
	MF_Combat           = (1<<6),
};
ENUM_CLASS_FLAGS(EItemSubobjDirtyFlags)

UCLASS(Abstract)
class ARPG_API ANAItemActor : public AActor, public INAInteractableInterface
{
	GENERATED_BODY()

	friend class UNAItemEngineSubsystem;
public:
	ANAItemActor(const FObjectInitializer& ObjectInitializer);

	virtual void OnConstruction(const FTransform& Transform) override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Item Actor")
	const UNAItemData* GetItemData() const;

	UFUNCTION(BlueprintCallable, Category = "Item Actor")
	bool HasValidItemID() const;
	
protected:
	// OnItemDataInitialized: BP 확장 가능
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnItemDataInitialized();
	virtual void OnItemDataInitialized_Implementation();
	
	virtual EItemSubobjDirtyFlags CheckDirtySubobjectFlags(const FNAItemBaseTableRow* MetaData) const;

private:
	void InitItemData();
	void VerifyInteractableData();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Actor | Root Shape")
	TObjectPtr<UShapeComponent> ItemRootShape;
	
	UPROPERTY(VisibleAnywhere, Category = "Item Actor | Mesh")
	TObjectPtr<UMeshComponent> ItemMesh;

	UPROPERTY(VisibleAnywhere, Category = "Item Actor | Static Mesh")
	TObjectPtr<class UGeometryCollection> ItemFractureCollection;
	
	UPROPERTY(VisibleAnywhere, Category = "Item Actor | Static Mesh")
	TObjectPtr<class UGeometryCollectionCache> ItemFractureCache;

	UPROPERTY(VisibleAnywhere, Category="Item Actor | Interaction Button")
	TObjectPtr<UBillboardComponent> ItemInteractionButton;

	UPROPERTY(VisibleAnywhere, Category="Item Actor | Interaction Button")
	TObjectPtr<UTextRenderComponent> ItemInteractionButtonText;

private:
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Item Actor", meta = (AllowPrivateAccess = "true"))
	FName ItemDataID;

	//======================================================================================================================
	// Interactable Interface Implements
	//======================================================================================================================
public:
	virtual FNAInteractableData GetInteractableData_Implementation() const override;
	virtual bool GetInteractableData_Internal(FNAInteractableData& OutIxData) const override;
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
};

