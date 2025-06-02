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
	FText Name = FText::GetEmpty();

	UPROPERTY(EditAnywhere, Category = "Item Text Data")
	FText Description = FText::GetEmpty();

	UPROPERTY(EditAnywhere, Category = "Item Text Data")
	FText UsageText = FText::GetEmpty();
};

class UGeometryCollection;
class UGeometryCollectionCache;
USTRUCT()
struct ARPG_API FNAStaticMeshItemAssetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Static Mesh Item Asset Data")
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;

	// (선택) Fracture Geometry Collection 에셋
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

	// @TODO: 아이템 전용 애님 클래스 만들기?
	UPROPERTY(EditAnywhere, Category = "Skeletal Mesh Item Asset Data")
	TSubclassOf<UAnimInstance/*UItemAnimInstance*/> AnimClass;
};

UENUM(BlueprintType)
enum class EItemRootShapeType : uint8
{
	IRT_None		UMETA(Hidden),
	
	IRT_Sphere		UMETA(DisplayName = "Sphere"),
	IRT_Box			UMETA(DisplayName = "Box"),
	IRT_Capsule		UMETA(DisplayName = "Capsule"),
};

UENUM(BlueprintType)
enum class EItemMeshType : uint8
{
	IMT_None		UMETA(Hidden),
	
	IMT_Static		UMETA(DisplayName = "Static"),
	IMT_Skeletal	UMETA(DisplayName = "Skeletal"),
};

// Weight Capacity: 인벤토리에 적재 가능한 최대 아이템 스택 계산에 사용됨. 아이템의 스택 = 아이템 무게 * 아이템 수량
USTRUCT()
struct FItemNumericData
{
	GENERATED_BODY()

	//UPROPERTY(EditAnywhere, Category = "Item Numeric Data")
	//float ItemWeight = 0.0f;	// 아이템 무게, 아이템 스택 계산에 사용

	/**
	 * 인벤토리의 슬롯 1칸에 들어갈 수 있는 최대 수량
	 * 1이면 인벤토리 슬롯 1칸에 1개만 들어감
	 * 주의! MaxSlotStackSize은  MaxInventoryStackSize보다 큰 값을 가질 수 없음!
	 */
	UPROPERTY(EditAnywhere, Category = "Item Numeric Data", meta = (UIMin = 1, UIMax = 999))
	int32 MaxSlotStackSize = -1;

	/**
	* 인벤토리에 소지 가능한 최대 수량
	* 1이면 인벤토리에 1개만 들어감, 0이면 수량 제한 없음
	*/
	UPROPERTY(EditAnywhere, Category = "Item Numeric Data", meta = (UIMin = 0))
	int32 MaxInventoryHoldCount = -1;
	
	UPROPERTY(EditAnywhere, Category = "Item Numeric Data")
	uint8 bIsStackable : 1 = false;	// 인벤토리에 슬롯에 스택이 가능한지
};

USTRUCT()
struct FNAIconAssetData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Item Icon Asset Data")
	TObjectPtr<UTexture2D> Icon = nullptr;
};

USTRUCT()
struct FNACachedTransform
{
	GENERATED_BODY()
	
// Size ////////////////////////////////////////////////////////////////
	
	UPROPERTY()
	float RootSphereRadius = 0.f;

	UPROPERTY()
	FVector RootBoxExtent = FVector::ZeroVector;

	// X = Radius, Y = Half Height
	UPROPERTY()
	FVector2D RootCapsuleSize = FVector2D::ZeroVector;
	
// Relative Transform ////////////////////////////////////////////////////////////////

	UPROPERTY()
	FTransform RootTransform = FTransform::Identity;
	
	UPROPERTY()
	FTransform MeshTransform = FTransform::Identity;

	UPROPERTY()
	FTransform ButtonTransform = FTransform::Identity;
	
	UPROPERTY()
	FTransform ButtonTextTransform = FTransform::Identity;
};

class ANAItemActor;
USTRUCT()
struct ARPG_API FNAItemBaseTableRow : public FTableRowBase
{
	GENERATED_BODY()


	FNAItemBaseTableRow(UClass* InItemClass = nullptr);
	
	// meta=(BlueprintBaseOnly): 블루프린트 클래스만 노출하도록 강제하는 지정자인데, 5.6 미만에서 버그 있음(언리얼피셜)
	// https://forums.unrealengine.com/t/uproperty-specifier-blueprintbaseonly-is-not-working-this-is-clearly-bug/2334795/6
	// https://issues.unrealengine.com/issue/UE-210088
	// 휴먼 에러 주의(c++ 네이티브 클래스 선택하지 말 것)
	UPROPERTY(EditAnywhere, Category = "Item Base Data", meta=(BlueprintBaseOnly, AllowAbstract="false"))
	TSubclassOf<ANAItemActor> ItemClass = nullptr;

	UPROPERTY(EditAnywhere, Category ="Item Base Data")
	EItemType ItemType = EItemType::IT_None;

	UPROPERTY(EditAnywhere, Category = "Item Base Data")
	EItemRootShapeType RootShapeType = EItemRootShapeType::IRT_Sphere;

	/* ANAItemActor의 메쉬 타입*/
	UPROPERTY(EditAnywhere, Category = "Item Base Data")
	EItemMeshType MeshType = EItemMeshType::IMT_Static;
	
	/** Static Mesh 에셋, 2D 아이콘 텍스처 포함 */
	UPROPERTY(EditAnywhere, Category = "Item Base Data",
		meta=(EditCondition="MeshType==EItemMeshType::IMT_Static", EditConditionHides))
	FNAStaticMeshItemAssetData StaticMeshAssetData;

	/** Skeletal  Mesh  에셋, 2D 아이콘 텍스처 포함 */
	UPROPERTY(EditAnywhere, Category = "Item Base Data",
		meta=(EditCondition="MeshType==EItemMeshType::IMT_Skeletal", EditConditionHides))
	FNASkeletalMeshItemAssetData SkeletalMeshAssetData;
	
	UPROPERTY(EditAnywhere, Category ="Item Base Data")
	FNAIconAssetData IconAssetData;
	
	UPROPERTY(EditAnywhere, Category = "Item Base Data")
	FItemTextData TextData;

	UPROPERTY(EditAnywhere, Category = "Item Base Data")
	FItemNumericData NumericData;

protected:
	friend class INAInteractableInterface;
	UPROPERTY(EditAnywhere,/* Category = "Item Interactable Data", */meta=(ShowOnlyInnerProperties))
	FNAInteractableData InteractableData;

public:
	// 서브 오브젝트 트랜스폼 캐싱
	// 직접 편집 ㄴㄴ
	UPROPERTY(meta=(TextExportTransient))
	FNACachedTransform CachedTransforms;

protected:
	virtual void OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName) override;
};

