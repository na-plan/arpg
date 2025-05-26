#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Interaction/NAInteractableDataStructs.h"
#include "NAItemBaseDataStructs.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	IT_None                UMETA(DisplayName = "None"),		// 초기화 필요

	IT_Tool                UMETA(DisplayName = "Tool"),
	IT_Weapon          UMETA(DisplayName = "Weapon"),
	IT_Ammo          UMETA(DisplayName = "Ammo"),
	IT_Armor               UMETA(DisplayName = "Armor"),
	IT_UpgradeItem               UMETA(DisplayName = "Upgrade Item"),
	IT_Consumable      UMETA(DisplayName = "Consumable"),
	IT_QuestItem           UMETA(DisplayName = "Quest Item"),
	IT_Material                UMETA(DisplayName = "Material"),
	IT_Currency            UMETA(DisplayName = "Currency"),
	IT_KeyItem             UMETA(DisplayName = "Key Item"),
	IT_Misc                UMETA(DisplayName = "Miscellaneous"),
};

USTRUCT()
struct FItemTextData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Item Text Data")
	FText Name;

	UPROPERTY(EditAnywhere, Category = "Item Text Data")
	FText Description;

	UPROPERTY(EditAnywhere, Category = "Item Text Data")
	FText InteractionText;

	UPROPERTY(EditAnywhere, Category = "Item Text Data")
	FText UsageText;
};

class UGeometryCollection;
class UGeometryCollectionCache;
USTRUCT()
struct ARPG_API FNAStaticMeshItemAssetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Static Mesh Item Asset Data")
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "Static Mesh Item Asset Data")
	FTransform MeshTransform = FTransform::Identity;

	// (선택) Fracture  Geometry Collection 에셋
	UPROPERTY(EditAnywhere, Category = "Static Mesh Item Asset Data|Fracture")
	UGeometryCollection* FractureCollection = nullptr;

	// (선택) Fracture Geometry Collection 애니메이션 
	UPROPERTY(EditAnywhere, Category = "Static Mesh Item Asset Data|Fracture")
	UGeometryCollectionCache* FractureCache = nullptr;
};

USTRUCT()
struct FNASkeletalMeshItemAssetData
{
	GENERATED_BODY()
	

	UPROPERTY(EditAnywhere, Category = "Skeletal Mesh Item Asset Data")
	TObjectPtr<USkeletalMesh> SkeletalMesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "Skeletal Mesh Item Asset Data")
	FTransform MeshTransform = FTransform::Identity;

	// @TODO: 아이템 전용 애님 클래스 만들기
	UPROPERTY(EditAnywhere, Category = "Skeletal Mesh Item Asset Data")
	TSubclassOf<UAnimInstance/*UItemAnimInstance*/> AnimClass;
};

UENUM(BlueprintType)
enum class EItemRootShapeType : uint8
{
	IRT_None		UMETA(DisplayName = "None"),

	IRT_Sphere		UMETA(DisplayName = "Sphere"),
	IRT_Box			UMETA(DisplayName = "Box"),
	IRT_Capsule		UMETA(DisplayName = "Capsule"),
};

UENUM(BlueprintType)
enum class EItemMeshType : uint8
{
	IMT_None		UMETA(DisplayName = "None"),

	IMT_Static		UMETA(DisplayName = "Static"),
	IMT_Skeletal UMETA(DisplayName = "Skeletal"),
};

// Weight Capacity: 인벤토리에 적재 가능한 최대 아이템 스택 계산에 사용됨. 아이템의 스택 = 아이템 무게 * 아이템 수량
USTRUCT()
struct FItemNumericData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Item Numeric Data")
	float ItemWeight = 0.0f;	// 아이템 무게, 아이템 스택 계산에 사용

	/**
	 * MaxStackSize가 0이면 수량 제한 없음, 1이면 인벤토리 슬롯 1칸에 1개만 들어감
	 * 주의! MaxSlotStackSize은  MaxInventoryStackSize보다 큰 값을 가질 수 없음!
	 */
	UPROPERTY(EditAnywhere, Category = "Item Numeric Data", meta = (UIMin = 0, UIMax = 999))
	int32 MaxSlotStackSize = -1;
	
	UPROPERTY(EditAnywhere, Category = "Item Numeric Data", meta = (UIMin = 0))
	int32 MaxInventoryStackSize = -1; // 인벤토리에 소지 가능한 최대 수량, 0이면 수량 제한 없음
	
	UPROPERTY(EditAnywhere, Category = "Item Numeric Data")
	uint8 bIsStackable : 1 = false;	// 인벤토리에 소지 가능 여부
};

USTRUCT()
struct FNAIconAssetData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Item Icon Asset Data")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, Category = "Item Icon Asset Data")
	FTransform IconTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, Category = "Item Icon Asset Data")
	FTransform IconTextTransform = FTransform::Identity;
};

class ANAItemActor;
USTRUCT()
struct ARPG_API FNAItemBaseTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Item Base Data")
	TSubclassOf<ANAItemActor> ItemClass;

	UPROPERTY(EditAnywhere, Category = "Item Base Data")
	uint8 bIsPickable : 1 = false;

	UPROPERTY(EditAnywhere, Category = "Item Base Data")
	EItemRootShapeType RootShapeType = EItemRootShapeType::IRT_Sphere;

	UPROPERTY(EditAnywhere, Category = "Item Base Data",
		meta = (EditCondition = "RootShapeType==EItemRootShapeType::IRT_Sphere", EditConditionHides, ClampMin = "0.0"))
	float RootSphereRadius = 20.f;

	UPROPERTY(EditAnywhere, Category = "Item Base Data",
		meta = (EditCondition = "RootShapeType==EItemRootShapeType::IRT_Box", EditConditionHides))
	FVector RootBoxExtent = FVector(20.f, 20.f, 20.f);

	UPROPERTY(EditAnywhere, Category = "Item Base Data",
		meta = (EditCondition = "RootShapeType==EItemRootShapeType::IRT_Capsule", EditConditionHides, ClampMin = "0.0"))
	float RootCapsuleHalfHeight = 20.f;

	UPROPERTY(EditAnywhere, Category = "Item Base Data",
		meta = (EditCondition = "RootShapeType==EItemRootShapeType::IRT_Capsule", EditConditionHides, ClampMin = "0.0"))
	float RootCapsuleRadius = 20.f;

	UPROPERTY(EditAnywhere, Category = "Item Base Data")
	FTransform RootShapeTransform = FTransform::Identity;

	/* ANAItemActor의 메쉬 타입*/
	UPROPERTY(EditAnywhere, Category = "Item Base Data")
	EItemMeshType MeshType = EItemMeshType::IMT_Static;

	/** Static Mesh 에셋, 2D 아이콘 텍스처 포함 */
	UPROPERTY(EditAnywhere, Category = "Item Base Data",
		meta=(EditCondition="MeshType==EItemMeshType::IMT_Static", EditConditionHides/*, ShowOnlyInnerProperties*/))
	FNAStaticMeshItemAssetData StaticMeshAssetData;

	/** Skeletal  Mesh  에셋, 2D 아이콘 텍스처 포함 */
	UPROPERTY(EditAnywhere, Category = "Item Base Data",
		meta=(EditCondition="MeshType==EItemMeshType::IMT_Skeletal", EditConditionHides/*, ShowOnlyInnerProperties*/))
	FNASkeletalMeshItemAssetData SkeletalMeshAssetData;
	
	UPROPERTY(EditAnywhere, Category ="Item Base Data")
	FNAIconAssetData IconAssetData;
	
	UPROPERTY(EditAnywhere, Category ="Item Base Data")
	EItemType ItemType = EItemType::IT_None;

	UPROPERTY(EditAnywhere, Category = "Item Base Data")
	FItemTextData TextData;

	UPROPERTY(EditAnywhere, Category = "Item Base Data")
	FItemNumericData NumericData;

protected:
	friend class INAInteractableInterface;
	UPROPERTY(EditAnywhere, Category = "Item Base Data | Interactable")
	FNAInteractableData InteractableData;

protected:
	virtual void OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName) override;
};

