#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "NAItemBaseData.generated.h"

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

UENUM(BlueprintType)
enum class EItemState : uint8
{
	IS_None            UMETA(DisplayName = "None"),           // �ʱ�ȭ �ʿ�

	IS_Active          UMETA(DisplayName = "Active"),         // ��� ���� ����
	IS_Inactive        UMETA(DisplayName = "Inactive"),       // ��� �Ұ� (���� ������, ��ٿ� ��)
	IS_Consumed        UMETA(DisplayName = "Consumed"),       // ��� �� ����� ����
	IS_Disabled        UMETA(DisplayName = "Disabled"),       // �ǵ������� ��Ȱ��ȭ�� ����
	IS_Locked          UMETA(DisplayName = "Locked"),         // Ư�� ������ �����ؾ� ����� �� ����
	IS_Expired         UMETA(DisplayName = "Expired"),        // ��ȿ�Ⱓ ��� ������ ��ȿ ����
	IS_Hidden          UMETA(DisplayName = "Hidden"),          // UI ��� ���� ó��
	IS_Equipped          UMETA(DisplayName = "Equipped"),          // ���� ���� ��� ������ ����
	IS_Broken          UMETA(DisplayName = "Broken"),          // �������� 0�� �Ǿ� �۵����� �ʴ� ����
	IS_Pending          UMETA(DisplayName = "Pending"),          // ��Ʈ��ũ ó�� ���̰ų�, UI ���� �� ��� ��� ����
	IS_PreviewOnly          UMETA(DisplayName = "PreviewOnly"),          // ���忡 ������ ��ȣ�ۿ��� �� ���� ���� (��: ���� ������)
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

	// (����) Fracture ������ �ʿ��� Geometry Collection ����
	UPROPERTY(EditAnywhere, Category = "Static Mesh Item Asset Data|Fracture")
	TObjectPtr<UGeometryCollection> FractureCollection = nullptr;

	// (����) Fracture �ִϸ��̼� 
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
	int32 MaxStackSize = -1;	// 0 <= MaxStackSize �̸� �κ��丮�� �� ����
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

	/** � Ÿ���� �޽� ������ ���� ���� �Ǻ��� */
	UPROPERTY(EditAnywhere, Category = "Item Base")
	EItemMeshType ItemMeshType = EItemMeshType::IMT_None;

	/** Static Mesh ���� ������ */
	UPROPERTY(EditAnywhere, Category = "Item Base",
		meta=(EditCondition="ItemMeshType==EItemMeshType::IMT_Static", EditConditionHides/*, ShowOnlyInnerProperties*/))
	FNAStaticMeshItemAssetData StaticMeshItemAssetData;

	/** Skeletal  Mesh ���� ������ */
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

