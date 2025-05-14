#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "NAItemBaseData.generated.h"

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

UENUM(BlueprintType)
enum class EItemState : uint8
{
	IS_None            UMETA(DisplayName = "None"),           // 초기화 필요

	IS_Active          UMETA(DisplayName = "Active"),         // 사용 가능 상태
	IS_Inactive        UMETA(DisplayName = "Inactive"),       // 사용 불가 (조건 불충족, 쿨다운 등)
	IS_Consumed        UMETA(DisplayName = "Consumed"),       // 사용 후 사라진 상태
	IS_Disabled        UMETA(DisplayName = "Disabled"),       // 의도적으로 비활성화된 상태
	IS_Locked          UMETA(DisplayName = "Locked"),         // 특정 조건을 만족해야 사용할 수 있음
	IS_Expired         UMETA(DisplayName = "Expired"),        // 유효기간 경과 등으로 무효 상태
	IS_Hidden          UMETA(DisplayName = "Hidden"),          // UI 등에서 숨김 처리
	IS_Equipped          UMETA(DisplayName = "Equipped"),          // 착용 중인 장비 아이템 전용
	IS_Broken          UMETA(DisplayName = "Broken"),          // 내구도가 0이 되어 작동하지 않는 상태
	IS_Pending          UMETA(DisplayName = "Pending"),          // 네트워크 처리 중이거나, UI 연출 중 잠시 대기 상태
	IS_PreviewOnly          UMETA(DisplayName = "PreviewOnly"),          // 월드에 있지만 상호작용할 수 없는 상태 (예: 상점 프리뷰)
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

	// (선택) Fracture 구현에 필요한 Geometry Collection 에셋
	UPROPERTY(EditAnywhere, Category = "Static Mesh Item Asset Data|Fracture")
	TObjectPtr<UGeometryCollection> FractureCollection = nullptr;

	// (선택) Fracture 애니메이션 
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
enum class EItemMeshType : uint8
{
	IMT_None		UMETA(DisplayName = "None"),

	IMT_Static		UMETA(DisplayName = "Static"),
	IMT_Skeletal UMETA(DisplayName = "Skeletal"),
};

USTRUCT()
struct FItemNumericData
{
	GENERATED_BODY()

	int32 ItemId = -1;
	int32 MaxStackSize = -1;	// 0 <= MaxStackSize 이면 인벤토리에 못 넣음
	float Weight = 0.0f;	
};

class ANAItemBase;
USTRUCT()
struct ARPG_API FNAItemBaseTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Item Base")
	uint8 bUseTriggerShpereAsRoot : 1 = true;

	UPROPERTY(EditAnywhere, Category = "Item Base",
		meta = (EditCondition = "bUseTriggerShpereAsRoot", EditConditionHides))
	FTransform TriggerSphereTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, Category = "Item Base",
		meta = (EditCondition = "bUseTriggerShpereAsRoot", EditConditionHides, ClampMin = "0.0"))
	float TriggerSphereRadius = 0.0f;

	/** 어떤 타입의 메시 에셋을 쓸지 고르는 판별자 */
	UPROPERTY(EditAnywhere, Category = "Item Base")
	EItemMeshType ItemMeshType = EItemMeshType::IMT_None;

	/** Static Mesh 전용 데이터 */
	UPROPERTY(EditAnywhere, Category = "Item Base",
		meta=(EditCondition="ItemMeshType==EItemMeshType::IMT_Static", EditConditionHides/*, ShowOnlyInnerProperties*/))
	FNAStaticMeshItemAssetData StaticMeshItemAssetData;

	/** Skeletal  Mesh 전용 데이터 */
	UPROPERTY(EditAnywhere, Category = "Item Base",
		meta=(EditCondition="ItemMeshType==EItemMeshType::IMT_Skeletal", EditConditionHides/*, ShowOnlyInnerProperties*/))
	FNASkeletalMeshItemAssetData SkeletalMeshItemAssetData;

	UPROPERTY(EditAnywhere, Category = "Item Base")
	TSubclassOf<ANAItemBase> ItemClass;

	UPROPERTY(EditAnywhere, Category ="Item Base")
	EItemType ItemType = EItemType::IT_None;

	UPROPERTY(EditAnywhere, Category = "Item Base")
	FItemTextData ItemTextData;

	UPROPERTY(EditAnywhere, Category = "Item Base")
	FItemNumericData ItemNumericData;

	UPROPERTY(EditAnywhere, Category = "Item Base")
	EItemState ItemState = EItemState::IS_None;
};

