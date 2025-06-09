// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/NAInteractionComponent.h"

#include "NACharacter.h"
#include "Item/ItemActor/NAItemActor.h"
#include "Inventory/NAInventoryComponent.h"


// Sets default values for this component's properties
UNAInteractionComponent::UNAInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetComponentTickEnabled(false);

	// ...
}

void UNAInteractionComponent::SetUpdate( const bool bFlag )
{
	bUpdateInteractionData = bFlag;
}

void UNAInteractionComponent::OnRegister()
{
	Super::OnRegister();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	if (!GetOwner())
	{
		return;
	}

	// Owner의 오버랩 및 콜리전 설정 재점검: 상호작용 기능 활성화를 위해
	if (APawn* OwningPawn = Cast<APawn>(GetOwner()))
	{
		if (UPrimitiveComponent* OwnerRootPrimitive = Cast<UPrimitiveComponent>(OwningPawn->GetRootComponent()))
		{
			OwnerRootPrimitive->SetGenerateOverlapEvents(true);
		}
	}
	else if (AController* OwningController = Cast<AController>(GetOwner()))
	{
		if (APawn* PossessingPawn = OwningController->GetPawn())
		{
			if (UPrimitiveComponent* OwnerRootPrimitive = Cast<UPrimitiveComponent>(PossessingPawn->GetRootComponent()))
			{
				OwnerRootPrimitive->SetGenerateOverlapEvents(true);
			}
		}
	}
	
}

// void UNAInteractionComponent::OnActorBeginOverlap( AActor* /*OverlappedActor*/, AActor* OtherActor )
// {
// 	if ( const TScriptInterface<INAInteractableInterface>& Interface = OtherActor )
// 	{
// 		OnInteractableFound( Interface );	
// 	}
// }

// void UNAInteractionComponent::OnActorEndOverlap( AActor* OverlappedActor, AActor* OtherActor )
// {
// 	if ( const TScriptInterface<INAInteractableInterface>& Interface = OtherActor )
// 	{
// 		OnInteractableLost( Interface);	
// 	}
// }

// Called when the game starts
void UNAInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if ( AActor* InteractionActor = GetOwner();
		 InteractionActor && GetNetMode() != NM_Client )
	{
		//InteractionActor->OnActorBeginOverlap.AddUniqueDynamic( this, &UNAInteractionComponent::OnActorBeginOverlap );
		//InteractionActor->OnActorEndOverlap.AddUniqueDynamic( this, &UNAInteractionComponent::OnActorEndOverlap );
	}
}

// Called every frame
void UNAInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	CurrentInteractionCheckTime += DeltaTime;
	if (bUpdateInteractionData && CurrentInteractionCheckTime >= InteractionCheckFrequency)
	{
		UpdateInteractionData();
		CurrentInteractionCheckTime = 0.f;
	}
}

// 어우 길어
// 어태치되면서 기존 액터를 ChlidActorComponent가 생성한 객체로 대체
// ItemData & InteractionData & Interactable State 전부 인수인계함
/*AActor**/TScriptInterface<INAInteractableInterface> UNAInteractionComponent::TryAttachItemMeshToOwner(
	TScriptInterface<INAInteractableInterface> InteractableActor)
{
	if (ActiveInteractable != InteractableActor)
	{
		// 로그
		check( false );
		return nullptr;
	}

	if (UObject* ActiveInteractableObj = ActiveInteractable.GetRawObject())
	{
		ANAItemActor* ActiveInteractableInstance = Cast<ANAItemActor>(ActiveInteractableObj);
		if (!ActiveInteractableInstance)
		{
			// 로그
			check( false );
			return nullptr;
		}

		if ( const TScriptInterface<INAHandActor> HandActor = GetOwner() )
		{
			if ( !HandActor->GetRightHandChildActorComponent()->GetChildActor() )
			{
				HandActor->GetRightHandChildActorComponent()->SetChildActorClass( ActiveInteractableInstance->GetClass());
				if (AActor* NewRightHandChlidActor = HandActor->GetRightHandChildActorComponent()->GetChildActor())
				{
					FWeakInteractableHandle NewRightHandle(NewRightHandChlidActor); // 여기서 인터페이스 구현 여부, 유효성 검사 다 함
					if (ensureAlways(NewRightHandle.IsValid())) // 혹시 모르니깐 한 번 더...
					{
						ANAItemActor* NewlyAttachedItemActor = CastChecked<ANAItemActor>(NewRightHandle.GetRawObject());
						ANAItemActor::TransferItemDataToDuplicatedActor(ActiveInteractableInstance, NewlyAttachedItemActor);
						TransferInteractableMidInteraction(NewRightHandle);
						return NewRightHandle.ToScriptInterface();
					}
				}
				
				return nullptr;
			}
			
			if ( !HandActor->GetLeftHandChildActorComponent()->GetChildActor())
			{
				HandActor->GetLeftHandChildActorComponent()->SetChildActorClass( ActiveInteractableInstance->GetClass() );
				if (AActor* NewLeftHandChlidActor = HandActor->GetLeftHandChildActorComponent()->GetChildActor())
				{
					FWeakInteractableHandle NewLeftHandle(NewLeftHandChlidActor); // 여기서 인터페이스 구현 여부, 유효성 검사 다 함
					if (ensureAlways(NewLeftHandle.IsValid())) // 혹시 모르니깐 한 번 더...
					{
						ANAItemActor* NewlyAttachedItemActor = CastChecked<ANAItemActor>(NewLeftHandle.GetRawObject());
						ANAItemActor::TransferItemDataToDuplicatedActor(ActiveInteractableInstance, NewlyAttachedItemActor);
						TransferInteractableMidInteraction(NewLeftHandle);
						return NewLeftHandle.ToScriptInterface();
					}
				}
				return nullptr;
			}
		}
	}

	return nullptr;
}

void UNAInteractionComponent::TransferInteractableMidInteraction(FWeakInteractableHandle NewActiveInteractable)
{
	if (!ensure(NewActiveInteractable.IsValid())) return;

	FNAInteractionData CachedInteractableData;
	FocusedInteractableMap.RemoveAndCopyValue(ActiveInteractable, CachedInteractableData);
	if (CachedInteractableData.IsValid())
	{
		FocusedInteractableMap.Emplace(NewActiveInteractable, CachedInteractableData);
		ActiveInteractable = NewActiveInteractable;
	}
}

void UNAInteractionComponent::UpdateInteractionData()
{
	if (!GetOwner()) { return;}
	
	if (FocusedInteractableMap.IsEmpty())
	{
		bUpdateInteractionData = false;
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	float MinDistance = TNumericLimits<float>::Max();
	TWeakInterfacePtr<INAInteractableInterface> BestInteractable;

	for (auto& It : FocusedInteractableMap)
	{
		if (!It.Key.IsValid())
		{
			continue;
		}
		if (It.Key.GetRawObject() != It.Value.FocusedInteractable.Get())
		{
			continue;
		}

		It.Value.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();
		FVector OwnerLoc;
		if (APawn* OwningPawn = Cast<APawn>(GetOwner()))
		{
			OwnerLoc = OwningPawn->GetActorLocation();
		}
		else if (AController* OwningController = Cast<AController>(GetOwner()))
		{
			if (APawn* PossessingPawn = OwningController->GetPawn())
			{
				OwnerLoc = PossessingPawn->GetActorLocation();
			}
			else
			{
				// 에러 로그
				return;
			}
		}
		
		FVector InteractableLoc = It.Value.FocusedInteractable->GetActorLocation();
		It.Value.DistanceToActiveInteractable = FVector::Dist(OwnerLoc, InteractableLoc);

		// 최소 거리 인터랙터블 검사
		if (It.Value.DistanceToActiveInteractable < MinDistance)
		{
			MinDistance       = It.Value.DistanceToActiveInteractable;
			BestInteractable  = It.Key.ToWeakInterface();
		}
	}

	if (BestInteractable.IsValid())
	{
		// 루프 이후에 멤버 변수로 결과 저장
		SetNearestInteractable(BestInteractable.Get());
	}
}

void UNAInteractionComponent::SetNearestInteractable(INAInteractableInterface* InteractableActor)
{
	if (!NearestInteractable.IsValid() || NearestInteractable != InteractableActor)
	{
		NearestInteractable = InteractableActor;
		// 위젯에 출력할 Interactable이 바뀌었음을 알리기
	}
}

bool UNAInteractionComponent::OnInteractableFound(TScriptInterface<INAInteractableInterface> InteractableActor)
{
	if (!InteractableActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UNAInteractionComponent::OnInteractableFound]  InteractableActor이 null이었음"));
		return false;
	}

	// if (!FocusedInteractable.IsValid() || FocusedInteractable.Get() != InteractableActor)
	// {
	// 	FocusedInteractable = InteractableActor;
	// 	if (UObject* InteractableObj = FocusedInteractable.GetObject())
	// 	{
	// 		InteractionData.ActiveInteractable = Cast<ANAItemActor>(InteractableObj);
	// 	}
	// 	else
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("[UNAInteractionComponent::OnInteractableFound]  이 로그가 뜬거면 진짜 뭔가 이상한거임.. InteractableObj이 어째서 유효하지 않는게야"));
	// 		return;
	// 	}
	// 	SetComponentTickEnabled(true);
	// }
	if (FocusedInteractableMap.IsEmpty() || !FocusedInteractableMap.Contains(InteractableActor))
	{
		UObject* InteractableObj = InteractableActor.GetObject();
		if (!InteractableObj)
		{
			UE_LOG(LogTemp, Warning, TEXT("[UNAInteractionComponent::OnInteractableFound]  이 로그가 뜬거면 진짜 뭔가 이상한거임.. 어째서 InteractableObj가 유효하지 않는게야"));
			return false;
		}
		ANAItemActor* InteractableItem = Cast<ANAItemActor>(InteractableObj);
		if (!InteractableItem)
		{
			UE_LOG(LogTemp, Warning, TEXT("[UNAInteractionComponent::OnInteractableFound]  이 로그가 뜬거면 진짜 뭔가 이상한거임.. 어째서 ANAItemActor 캐스팅에 실패한게야"));
			return false;
		}
		FNAInteractionData NewInteractionData;
		NewInteractionData.FocusedInteractable = InteractableItem;

		FocusedInteractableMap.Emplace(InteractableActor, NewInteractionData);

		if (!IsComponentTickEnabled())
		{
			SetComponentTickEnabled(true);
			UpdateInteractionData(); // 틱 지연 방지
		}
		if (!bUpdateInteractionData)
		{
			bUpdateInteractionData = true;
		}
	}

	return true;
}

bool UNAInteractionComponent::OnInteractableLost(TScriptInterface<INAInteractableInterface> InteractableActor)
{
	if (!InteractableActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UNAInteractionComponent::OnInteractableLost]  InteractableActor이 null이었음"));
		return false;
	}

	if (FocusedInteractableMap.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UNAInteractionComponent::OnInteractableLost]  FocusedInteractableMap이 비어있었는데??"));
		return false;
	}

	if (!FocusedInteractableMap.Contains(InteractableActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UNAInteractionComponent::OnInteractableLost]  FocusedInteractableMap에 InteractableActor이 없었음"));
		return false;
	}

	int32 bSucceed = FocusedInteractableMap.Remove(InteractableActor);
	if (bSucceed != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UNAInteractionComponent::OnInteractableLost]  FocusedInteractableMap에서 InteractableActor 제거 실패: Removed %d"), bSucceed);
		return false;
	}

	if (ActiveInteractable == InteractableActor
		&& InteractableActor->Execute_IsOnInteract(InteractableActor.GetObject()))
	{
		// 인터렉션 중인 액터인데 오버랩이 끊겼다고????
		return false;
	}

	if (ActiveInteractable == InteractableActor)
	{
		ActiveInteractable = nullptr;
	}
	if (NearestInteractable == InteractableActor)
	{
		NearestInteractable = nullptr;
	}

	return true;
}

void UNAInteractionComponent::StartInteraction(/*INAInteractableInterface* InteractableActor*/)
{
	if (!NearestInteractable.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UNAInteractionComponent::StartInteraction]  NearestInteractable이 유효하지 않음"));
		return;
	}

	ActiveInteractable = NearestInteractable;
	ActiveInteractable.ToWeakInterface()->Execute_BeginInteract(ActiveInteractable.GetRawObject(), GetOwner());
	if (ActiveInteractable.IsValid()
		&& ActiveInteractable.ToWeakInterface()->Execute_IsOnInteract(ActiveInteractable.GetRawObject()))
	{
		bHasPendingUseItem = true;
	}
}

void UNAInteractionComponent::StopInteraction(/*TScriptInterface<INAInteractableInterface> InteractableActor*/)
{
	if (!ensure(bHasPendingUseItem && ActiveInteractable.IsValid())) return;

	ActiveInteractable.ToScriptInterface()->Execute_EndInteract(ActiveInteractable.GetRawObject(), GetOwner());
	
	// 어태치된 액터가 있었다면..??? -> 이건 Interactable에서 처리하기
}

void UNAInteractionComponent::OnInteractionEnded(TScriptInterface<INAInteractableInterface> InteractableActor)
{
	if (!InteractableActor
		   || ActiveInteractable.ToScriptInterface() != InteractableActor)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[UNAInteractionComponent::OnInteractionEnded]  상호작용 중인 Interactable(%s)과 상호작용을 멈추려던 Interactable(%s)이 서로 다른 객체였음")
			, *ActiveInteractable.GetRawObject()->GetName(), *InteractableActor.GetObject()->GetName());
		return;
	}

	ActiveInteractable = nullptr;
	bHasPendingUseItem = false;
}

// void UNAInteractionComponent::EndInteraction(/*INAInteractableInterface* InteractableActor*/)
// {
// 	if (!NearestInteractable.IsValid())
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("[UNAInteractionComponent::EndInteraction]  NearestInteractable이 유효하지 않음"));
// 		return;
// 	}
//
// 	ActiveInteractable.ToWeakInterface()->Execute_EndInteract(ActiveInteractable.GetRawObject(), GetOwner());
// 	ActiveInteractable = nullptr;
// }

// void UNAInteractionComponent::ExecuteInteraction(/*INAInteractableInterface* InteractableActor*/)
// {
// 	if (!ActiveInteractable.IsValid())
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("[UNAInteractionComponent::ExecuteInteraction]  ActiveInteractable 유효하지 않음"));
// 		return;
// 	}
//
// 	ActiveInteractable.ToWeakInterface()->Execute_ExecuteInteract(ActiveInteractable.GetRawObject(), GetOwner());
// }

bool UNAInteractionComponent::TryAddItemToInventory(ANAItemActor* ItemActor)
{
	UNAInventoryComponent* InventoryComp = GetOwner()->GetComponentByClass<UNAInventoryComponent>();
	if (!ensureAlways(InventoryComp)) return false;
	
	if (!ItemActor) return false;
	UNAItemData* Item = ItemActor->GetItemData();
	if (!Item) return false;
	
	int32 RemainQuantity =  InventoryComp->TryAddItem(Item);
	if (RemainQuantity == 0) // 전부 추가 성공
	{
		return true;
	}
	if (RemainQuantity > 0) // 부분 추가
	{
		// 아이템 액터 자체는 잔존하나, 아이템 데이터가 복제됨
		// 해당 아이템 액터에 남아있는 아이템 데이터 / 인벤토리에 적재된 아이템 데이터
	}
	else if (RemainQuantity == -1) // 전부 실패
	{
		// 아이템 액터 잔존 및 아이템 데이터 변경 없음
	}
	return false;
}
