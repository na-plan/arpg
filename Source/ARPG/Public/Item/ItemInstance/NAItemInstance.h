#pragma once

#include "GameFramework/Actor.h"
#include "Item/GameInstance/NAItemGameInstanceSubsystem.h"
#include "NAItemInstance.generated.h"

UCLASS()
class ARPG_API ANAItemInstance : public AActor
{
	GENERATED_BODY()

public:
	ANAItemInstance(const FObjectInitializer& ObjectInitializer);
	virtual void PostInitProperties() override;

	UFUNCTION(BlueprintCallable, Category = "Item Instance")
	UNAItemData* GetItemData() const;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual bool InitItemInstance(const FDataTableRowHandle& InDataTableRowHandle);
	
	template<typename ItemDTRow_T = FNAItemBaseTableRow>
		requires TIsDerivedFrom<ItemDTRow_T, FNAItemBaseTableRow>::IsDerived
	bool TryCreateItemData() noexcept;

	virtual void InitItemData_Impl();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnItemDataInitialized();
	virtual void OnItemDataInitialized_Implementation();

private:
	void InitItemData_Internal();

public:
	UPROPERTY(EditAnywhere, Category = "Item Instance")
	FDataTableRowHandle ItemDataTableRowHandle;
	
protected:
	// 기본값: USphereComponent::StaticClass
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Instance | Root Shape")
	TSubclassOf<UShapeComponent> ItemRootShapeClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Instance | Root Shape")
	TObjectPtr<class UShapeComponent> ItemRootShape = nullptr;

	UPROPERTY(VisibleAnywhere, Category="Item Instance | Mesh")
	TSubclassOf<UMeshComponent> ItemMeshClass;

	// Static Mesh가 기본값으로 설정됨
	UPROPERTY(VisibleAnywhere, Category = "Item Instance | Mesh")
	TObjectPtr<UStaticMeshComponent> ItemStaticMesh;

	// �ʿ�� SetData���� ������, �⺻������ ������ ItemStaticMesh�� ��Ȱ��ȭ
	UPROPERTY(VisibleAnywhere, Category = "Item Instance | Mesh")
	TObjectPtr<USkeletalMeshComponent> ItemSkeletalMesh = nullptr;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Instance", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<class UNAItemData> ItemData = nullptr;
};

template<typename ItemDTRow_T>
	requires TIsDerivedFrom<ItemDTRow_T, FNAItemBaseTableRow>::IsDerived
inline bool ANAItemInstance::TryCreateItemData() noexcept
{
	if (ItemData.IsExplicitlyNull())
	{
		if (UWorld* World = GetWorld()) {
			if (UGameInstance* GI = World->GetGameInstance()) {
				if (UNAItemGameInstanceSubsystem* ItemSubsys = GI->GetSubsystem<UNAItemGameInstanceSubsystem>()) {
					UNAItemData* NewItemData = ItemSubsys->CreateItemData<ItemDTRow_T>(this);
					if (NewItemData) {
						ItemData = NewItemData;
						return true;
					}
				}
			}
		}
	} // sh1t
	else {
		UE_LOG(LogTemp, Warning, TEXT("[ANAItemInstance::TryCreateItemData]  이미 초기화된 ItemData."));
	}

	return false;
}
