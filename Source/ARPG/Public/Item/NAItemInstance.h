#pragma once

#include "Item/NAItemData.h"
#include "NAItemInstance.generated.h"

UCLASS()
class ARPG_API ANAItemInstance : public AActor
{
	GENERATED_BODY()

public:
	ANAItemInstance(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Item")
	UNAItemData* GetItemData() const;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual bool InitItemInstance(const FDataTableRowHandle& InDataTableRowHandle);

public:
	UPROPERTY(EditAnywhere, Category = "Item Data")
	FDataTableRowHandle ItemDataTableRowHandle;
	
protected:
	// 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Trigger Sphere")
	TObjectPtr<class USphereComponent> TriggerSphere = nullptr;

	UPROPERTY(VisibleAnywhere, Category="Item Mesh")
	TSubclassOf<UMeshComponent> ItemMeshClass;

	// Static Mesh가 기본값으로 설정됨
	UPROPERTY(VisibleAnywhere, Category = "Item Mesh")
	TObjectPtr<UStaticMeshComponent> ItemStaticMesh;

	// �ʿ�� SetData���� ������, �⺻������ ������ ItemStaticMesh�� ��Ȱ��ȭ
	UPROPERTY(VisibleAnywhere, Category = "Item Mesh")
	TObjectPtr<USkeletalMeshComponent> ItemSkeletalMesh = nullptr;

private:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = TBCharacter, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<UNAItemData> ItemData = nullptr;

	//// ���� ������
	//UPROPERTY(VisibleAnywhere, Category = "Item Ownership")
	//TWeakObjectPtr<AActor> CurrentOwner = nullptr;	
};
