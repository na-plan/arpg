#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "NAItemBaseDataStructs.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	IT_None                UMETA(DisplayName = "None"),		// �ʱ�ȭ �ʿ�

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
struct FNAStaticMeshItemAssetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Static Mesh Item Asset Data")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, Category = "Static Mesh Item Asset Data")
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "Static Mesh Item Asset Data")
	FTransform MeshTransform = FTransform::Identity;

	// (선택) Fracture  Geometry Collection 에셋
	UPROPERTY(EditAnywhere, Category = "Static Mesh Item Asset Data|Fracture")
	TObjectPtr<UGeometryCollection> FractureCollection = nullptr;

	// (선택) Fracture Geometry Collection 애니메이션 
	UPROPERTY(EditAnywhere, Category = "Static Mesh Item Asset Data|Fracture")
	TObjectPtr<UGeometryCollectionCache> FractureCache = nullptr;
};

USTRUCT()
struct FNASkeletalMeshItemAssetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Skeletal Mesh Item Asset Data")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, Category = "Skeletal Mesh Item Asset Data")
	TObjectPtr<USkeletalMesh> SkeletalMesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "Skeletal Mesh Item Asset Data")
	FTransform MeshTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, Category = "Skeletal Mesh Item Asset Data")
	TSubclassOf<UAnimInstance/*UItemAnimInstance*/> AnimClass;
};

UENUM(BlueprintType)
enum class EItemRootShapeType : uint8
{
	IRT_None		UMETA(DisplayName = "None"),

	IRT_Sphere		UMETA(DisplayName = "Sphere"),
	IRT_Square		UMETA(DisplayName = "Square"),
	IRT_Capsule	UMETA(DisplayName = "Capsule"),
};

UENUM(BlueprintType)
enum class EItemMeshType : uint8
{
	IMT_None		UMETA(DisplayName = "None"),

	IMT_Static		UMETA(DisplayName = "Static"),
	IMT_Skeletal UMETA(DisplayName = "Skeletal"),
};

// Weight Capacity: �κ��丮���� ������ �� �ִ� �ִ� ����
// Total Weight: �κ��丮�� ���� ����. ���� ���� ���� ������ ������ ����
// ������ ����: ������ ���� * ������ ����
// ���� �Ұ����� �������� �׻� ������ 1�� ���ֵ�, �̷� ��� �������� ���� == ������ ����
USTRUCT()
struct FItemNumericData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Item Numeric Data")
	float ItemWeight = 0.0f;	// ������ ����

	UPROPERTY(EditAnywhere, Category = "Item Numeric Data", meta = (UIMin = 0, UIMax = 100))
	int32 MaxStackSize = -1;	// 0�� ��� �κ��丮���� ���� �Ұ�, 1�� ��� bIsStackable�� false�� ���ֵ�

	UPROPERTY(EditAnywhere, Category = "Item Numeric Data")
	uint8 bIsStackable : 1 = false;	// true�� �κ��丮���� ���� ����
};

class ANAItemInstance;
USTRUCT()
struct ARPG_API FNAItemBaseTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Item Base Data")
	TSubclassOf<ANAItemInstance> ItemClass;

	UPROPERTY(EditAnywhere, Category = "Item Base Data")
	uint8 bIsPickable : 1 = false;

	UPROPERTY(EditAnywhere, Category = "Item Base Data")
	EItemRootShapeType RootShapeType = EItemRootShapeType::IRT_Sphere;

	UPROPERTY(EditAnywhere, Category = "Item Base Data",
		meta = (EditCondition = "MeshType==EItemMeshType::IRT_Sphere", EditConditionHides, ClampMin = "0.0"))
	float RootSphereRadius = 20.f;

	UPROPERTY(EditAnywhere, Category = "Item Base Data",
		meta = (EditCondition = "MeshType==EItemMeshType::IRT_Square", EditConditionHides))
	FVector RootBoxExtent = FVector(20.f, 20.f, 20.f);

	UPROPERTY(EditAnywhere, Category = "Item Base Data",
		meta = (EditCondition = "MeshType==EItemMeshType::IRT_Capsule", EditConditionHides, ClampMin = "0.0"))
	float RootCapsuleHalfHegiht = 20.f;

	UPROPERTY(EditAnywhere, Category = "Item Base Data",
		meta = (EditCondition = "MeshType==EItemMeshType::IRT_Capsule", EditConditionHides, ClampMin = "0.0"))
	float RootCapsuleRadius = 20.f;

	UPROPERTY(EditAnywhere, Category = "Item Base Data")
	FTransform RootShapeTransform = FTransform::Identity;

	/** � Ÿ���� �޽� ������ ���� ���� �Ǻ��� */
	UPROPERTY(EditAnywhere, Category = "Item Base Data")
	EItemMeshType MeshType = EItemMeshType::IMT_Static;

	/** Static Mesh ���� ������ */
	UPROPERTY(EditAnywhere, Category = "Item Base Data",
		meta=(EditCondition="MeshType==EItemMeshType::IMT_Static", EditConditionHides/*, ShowOnlyInnerProperties*/))
	FNAStaticMeshItemAssetData StaticMeshAssetData;

	/** Skeletal  Mesh ���� ������ */
	UPROPERTY(EditAnywhere, Category = "Item Base Data",
		meta=(EditCondition="MeshType==EItemMeshType::IMT_Skeletal", EditConditionHides/*, ShowOnlyInnerProperties*/))
	FNASkeletalMeshItemAssetData SkeletalMeshAssetData;

	UPROPERTY(EditAnywhere, Category ="Item Base Data")
	EItemType ItemType = EItemType::IT_None;

	UPROPERTY(EditAnywhere, Category = "Item Base Data")
	FItemTextData TextData;

	UPROPERTY(EditAnywhere, Category = "Item Base Data")
	FItemNumericData NumericData;
};

