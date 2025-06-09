// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/NAWeakInteractableHandle.h"
#include "NAInteractionComponent.generated.h"

class ANAItemActor;

USTRUCT(BlueprintType)
struct FNAInteractionData
{
	GENERATED_BODY()
public:
	FNAInteractionData()
		: FocusedInteractable(nullptr),
		LastInteractionCheckTime(0.f),
		DistanceToActiveInteractable(0.f)
	{
	}

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Interaction Data")
	TWeakObjectPtr<ANAItemActor> FocusedInteractable;

	// 마지막 상호작용 확인이 이루어진 월드 시간(초)
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interaction Data")
	float LastInteractionCheckTime;

	// @TODO: FVector로 변경해서 거리 및 방향 정보도 저장할지 말지 생각해보기
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interaction Data")
	float DistanceToActiveInteractable;

	// 상호작용 트리거 시 딜레이가 필요하면 쓰기
	FTimerHandle InteractionTimerHandle;

	bool IsValid() const
	{
		return FocusedInteractable.IsValid();
	}
};

class INAInteractableInterface;
struct FWeakInteractableKey;

/**
 * @TODO: 소유주의 트랜스폼 정보를 가져올 때, 소유주가 APawn인 경우 vs AController인 경우 나눠서 대응해야 함
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_API UNAInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNAInteractionComponent();

	void SetUpdate(const bool bFlag);

protected:
	virtual void OnRegister() override;

	// UFUNCTION()
	// void OnActorBeginOverlap( AActor* OverlappedActor, AActor* OtherActor );
	
	// UFUNCTION()
	// void OnActorEndOverlap( AActor* OverlappedActor, AActor* OtherActor );
	
	// Called when the game starts
	virtual void BeginPlay() override;

	void UpdateInteractionData();
	void SetNearestInteractable(INAInteractableInterface* InteractableActor);

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	//==================================================================================================
	// Interaction Properties
	//==================================================================================================

	//UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Interaction Component")
	//TWeakInterfacePtr<class INAInteractableInterface> FocusedInteractable;

	//UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Interaction Component")
	//FNAInteractionData InteractionData;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Interaction Component")
	TMap<FWeakInteractableHandle, FNAInteractionData> FocusedInteractableMap;
	//TMap<TWeakInterfacePtr<class INAInteractableInterface>, FNAInteractionData> FocusedInteractableMap;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Interaction Component")
	float InteractionCheckFrequency = 0.1f;

	float CurrentInteractionCheckTime;

	//UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Interaction Component")
	//float InteractionCheckDistance;

	//FTimerHandle InteractionTimerHandle;
	
	uint8 bUpdateInteractionData : 1 = false;

private:
	// 상호작용이 활성 중인(+사용 대기 상태) Interactable 객체
	UPROPERTY(/*VisibleInstanceOnly, BlueprintReadOnly, Category = "Interaction Component"*/)
	FWeakInteractableHandle ActiveInteractable = nullptr;
	//TWeakInterfacePtr<INAInteractableInterface> ActiveInteractable = nullptr;

	// 가장 가까이 있는 Interactable 객체
	UPROPERTY(/*VisibleInstanceOnly, BlueprintReadOnly, Category = "Interaction Component"*/)
	FWeakInteractableHandle NearestInteractable = nullptr;
	//TWeakInterfacePtr<INAInteractableInterface> NearestInteractable = nullptr;

	// 사용 대기 중인 아이템이 있는 경우 true(ChlildActor가 생성된 경우)
	uint8 bHasPendingUseItem : 1 = false;

public:
	//==================================================================================================
	// Interaction Functions
	//==================================================================================================

	// Interactable 아이템 인스턴스 쪽에서 상호작용 범위를 체크(트리거 콜리전 활용)
	// 아이템 인스턴스에서 유저가 상호작용 범위 내에 들어왔는지 체크 후 캐릭터에게 상호작용이 가능함을 알림
	bool OnInteractableFound(TScriptInterface<INAInteractableInterface> InteractableActor);
	// @TODO: FocusedInteractableMap 요소 지연 삭제 고민해보기 -> 상호작용 아이템 포커스 갱신 때문에 병목 생기는지 확인하기
	bool OnInteractableLost(TScriptInterface<INAInteractableInterface> InteractableActor);

	// 캐릭터에서 상호작용 시작 이니시
	void StartInteraction(/*INAInteractableInterface* InteractableActor*/);
	// 캐릭터에서 상호작용 중단 이니시
	void StopInteraction(/*TScriptInterface<INAInteractableInterface> InteractableActor*/);

	// Interactable에서 상호작용 실행을 실패/중단/완료한 경우에 호출됨
	void OnInteractionEnded(TScriptInterface<INAInteractableInterface> InteractableActor);

	bool HasPendingUseItem() const
	{
		return bHasPendingUseItem
				&&  ActiveInteractable.IsValid()
				&& ActiveInteractable.ToRawInterface()->Execute_IsOnInteract(ActiveInteractable.GetRawObject());
	}

	TScriptInterface<INAInteractableInterface> GetCurrentActiveInteractable() const
	{
		return ActiveInteractable.IsValid() ?  ActiveInteractable.GetRawObject() : nullptr;
	}
	
	// @ return	새로 생성된 Interactable 객체
	// 어태치에 성공하면 기존 액터의 Destory를 다음 프레임에 예약해놓음
	/*AActor**/TScriptInterface<INAInteractableInterface> TryAttachItemMeshToOwner(TScriptInterface<INAInteractableInterface> InteractableActor);
	
protected:
	void TransferInteractableMidInteraction(FWeakInteractableHandle NewActiveInteractable);
	
public:
	//==================================================================================================
	// 인벤토리 연계
	//==================================================================================================

	UFUNCTION( Client, Reliable )
	void Client_AddItemToInventory(ANAItemActor* ItemActor);
	
	bool TryAddItemToInventory(ANAItemActor* ItemActor);
};