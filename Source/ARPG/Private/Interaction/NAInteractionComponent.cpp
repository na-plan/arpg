// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/NAInteractionComponent.h"

#include "NACharacter.h"
#include "Item/ItemActor/NAItemActor.h"
#include "Inventory/Component/NAInventoryComponent.h"
#include "Item/ItemActor/NAPickableItemActor.h"
#include "Item/ItemActor/NAPlaceableItemActor.h"


// Sets default values for this component's properties
UNAInteractionComponent::UNAInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetComponentTickEnabled(false);

	SetIsReplicatedByDefault( true );

	// ...
}

void UNAInteractionComponent::OnRegister()
{
	Super::OnRegister();
	
}

// Called when the game starts
void UNAInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
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
TScriptInterface<INAInteractableInterface> UNAInteractionComponent::TryAttachItemMeshToOwner(
	TScriptInterface<INAInteractableInterface> InteractableActor)
{
	if (NearestInteractable != InteractableActor)
	{
		// 로그
		check( false );
		return nullptr;
	}

	//if (UObject* ActiveInteractableObj = ActiveInteractable.GetRawObject())
	if (UObject* ActiveInteractableObj = NearestInteractable.GetRawObject())
	{
		ANAItemActor* ActiveInteractableActor = Cast<ANAItemActor>(ActiveInteractableObj);
		if (!ActiveInteractableActor)
		{
			// 로그
			check( false );
			return nullptr;
		}

		if ( const TScriptInterface<INAHandActor> HandActor = GetOwner() )
		{
			if ( !HandActor->GetRightHandChildActorComponent()->GetChildActor())
			{
				HandActor->GetRightHandChildActorComponent()->SetChildActorClass( ActiveInteractableActor->GetClass());
				if (AActor* NewRightHandChildActor = HandActor->GetRightHandChildActorComponent()->GetChildActor())
				{
					ANAItemActor* NewlyAttachedItemActor = CastChecked<ANAItemActor>(NewRightHandChildActor);
					ANAItemActor::TransferItemDataToDuplicatedActor(ActiveInteractableActor, NewlyAttachedItemActor);
					TScriptInterface<INAInteractableInterface> NewlyAttachInteractableActor =
						NewlyAttachedItemActor->GetInteractableInterface();
					TransferInteractableMidInteraction(NewlyAttachInteractableActor);
					SetActiveInteractable(NewlyAttachInteractableActor);
					return NewlyAttachInteractableActor;
				}
				
				return nullptr;
			}
			
			if ( !HandActor->GetLeftHandChildActorComponent()->GetChildActor())
			{
				HandActor->GetLeftHandChildActorComponent()->SetChildActorClass( ActiveInteractableActor->GetClass());
				if (AActor* NewLeftHandChildActor = HandActor->GetLeftHandChildActorComponent()->GetChildActor())
				{
					ANAItemActor* NewlyAttachedItemActor = CastChecked<ANAItemActor>(NewLeftHandChildActor);
					ANAItemActor::TransferItemDataToDuplicatedActor(ActiveInteractableActor, NewlyAttachedItemActor);
					TScriptInterface<INAInteractableInterface> NewlyAttachInteractableActor =
						NewlyAttachedItemActor->GetInteractableInterface();
					TransferInteractableMidInteraction(NewlyAttachInteractableActor);
					SetActiveInteractable(NewlyAttachInteractableActor);
					return NewlyAttachInteractableActor;
				}
				
				return nullptr;
			}
		}
	}

	return nullptr;
}

void UNAInteractionComponent::TransferInteractableMidInteraction(TScriptInterface<INAInteractableInterface> InteractableActor)
{
	if (!InteractableActor.GetObject()) return;
	if (NearestInteractable == InteractableActor) return;
	
	FWeakInteractableHandle NewInteractableHandle(InteractableActor);
	FNAInteractionData CachedInteractableData;
	FocusedInteractables.RemoveAndCopyValue(NearestInteractable, CachedInteractableData);
	CachedInteractableData.InteractorActor = GetOwner();
	CachedInteractableData.InteractableItemActor = CastChecked<ANAItemActor>(InteractableActor.GetObject());
	if (CachedInteractableData.IsValid())
	{
		FocusedInteractables.Emplace(NewInteractableHandle, CachedInteractableData);
	}
}

void UNAInteractionComponent::Client_AddItemToInventory_Implementation( ANAItemActor* ItemActor )
{
	// 서버에서는 성공했는데 클라이언트에서는 실패한 경우, 동기화가 뭔가 잘못됐을 가능성
	check( TryAddItemToInventory( ItemActor ) );
}

void UNAInteractionComponent::UpdateInteractionData()
{
	if (!GetOwner()) { return;}
	
	if (FocusedInteractables.IsEmpty())
	{
		bUpdateInteractionData = false;
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	TPair<FWeakInteractableHandle, float> BestInteractable(nullptr, -FLT_MAX);
	for ( auto It = FocusedInteractables.CreateIterator(); It; ++It )
	{
		if (!It->Key.IsValid() || !It->Value.IsValid())
		{
			//ensureAlways(false);
			It.RemoveCurrent();
			continue;
		}
		if (It->Value.InteractorActor.Get() != GetOwner())
		{
			//ensureAlways(false);
			It.RemoveCurrent();
			continue;
		}
		if (AActor* AttachParentActor = It->Value.InteractorActor->GetAttachParentActor())
		{
			//ensureAlways(AttachParentActor == GetOwner());
			if (AttachParentActor != GetOwner())
			{
				It.RemoveCurrent();
			}
			continue;
		}

		It->Value.LastInteractionCheckTime = Now;
		AActor* Owner = GetOwner();
		AActor* Interactable = It->Value.InteractableItemActor.Get();
		FVector OwnerForward = Owner->GetActorForwardVector();
		
		FVector OwnerLoc = Owner->GetActorLocation();
		FVector InteractableLoc = Interactable->GetActorLocation();

		float Distance = FVector::Dist(OwnerLoc, InteractableLoc);
		FVector Direction = (InteractableLoc - OwnerLoc).GetSafeNormal();
		
		FVector2D ToTarget2D = FVector2D(InteractableLoc - OwnerLoc).GetSafeNormal();
		FVector2D Forward2D = FVector2D(Owner->GetActorForwardVector()).GetSafeNormal();
		float DotWithView = FVector2D::DotProduct(Forward2D, ToTarget2D);
		
		float AngleDegrees = FMath::RadiansToDegrees(acosf(FMath::Clamp(DotWithView, -1.0f, 1.0f)));
		const bool bIsInViewAngle = FMath::IsWithinInclusive(AngleDegrees,
			- InteractionAngleDegrees * 0.5f, InteractionAngleDegrees * 0.5f);
		
		It->Value.SetInteractionCandidateData(Distance, Direction, DotWithView, AngleDegrees, bIsInViewAngle);
		const float InteractionScore = It->Value.CalculateInteractionScore();

		if (BestInteractable.Value < InteractionScore)
		{
			BestInteractable.Key = It->Key;
			BestInteractable.Value = InteractionScore;
		}
	}
	
	SetNearestInteractable(BestInteractable.Key);
}

void UNAInteractionComponent::SetNearestInteractable(FWeakInteractableHandle InteractableActor)
{
	if (NearestInteractable != InteractableActor)
	{
		NearestInteractable = InteractableActor;
	}
}

bool UNAInteractionComponent::OnInteractableFound(TScriptInterface<INAInteractableInterface> InteractableActor)
{
	if (!GetOwner()) return false;
	if (!InteractableActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OnInteractableFound]  InteractableActor이 null이었음"));
		return false;
	}
	
	if (!FocusedInteractables.Contains(InteractableActor))
	{
		UObject* InteractableObj = InteractableActor.GetObject();
		if (!InteractableObj)
		{
			UE_LOG(LogTemp, Warning, TEXT("[OnInteractableFound]  InteractableObj가 유효하지 않음"));
			return false;
		}
		ANAItemActor* InteractableItem = Cast<ANAItemActor>(InteractableObj);
		if (!InteractableItem)
		{
			UE_LOG(LogTemp, Warning, TEXT("[OnInteractableFound]  InteractableActor를 ANAItemActor로 캐스팅 실패"));
			return false;
		}
		AActor* AttachParentActor = InteractableItem->GetAttachParentActor();
		if (AttachParentActor && AttachParentActor != GetOwner())
		{
			UE_LOG(LogTemp, Log, TEXT("%hs: %s 아이템 주인이 이미 있음"), __FUNCTION__, *GetNameSafe(InteractableItem));
			return false;
		}
		
		FNAInteractionData NewInteractionData;
		NewInteractionData.InteractorActor = GetOwner();
		NewInteractionData.InteractableItemActor = InteractableItem;
		FocusedInteractables.Emplace(InteractableActor, NewInteractionData);

		if (!IsComponentTickEnabled())
		{
			SetComponentTickEnabled(true);
		}
		if (!bUpdateInteractionData)
		{
			bUpdateInteractionData = true;
		}
		return true;
	}

	return false;
}

bool UNAInteractionComponent::OnInteractableLost(TScriptInterface<INAInteractableInterface> InteractableActor)
{
	if (!InteractableActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OnInteractableLost]  InteractableActor이 null이었음"));
		return false;
	}
	
	if (FocusedInteractables.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[OnInteractableLost]  FocusedInteractableMap이 비어있었는데??"));
		return false;
	}

	if (!FocusedInteractables.Contains(InteractableActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("[OnInteractableLost]  FocusedInteractableMap에 없던 InteractableActor이었음"));
		return false;
	}

	int32 bSucceed = FocusedInteractables.Remove(InteractableActor);
	if (bSucceed != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT(
			"[OnInteractableLost]  FocusedInteractableMap에서 InteractableActor 제거 실패: Removed %d"), bSucceed);
		return false;
	}

	if (ActiveInteractable == InteractableActor
		&& InteractableActor->Execute_IsOnInteract(InteractableActor.GetObject()))
	{
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

void UNAInteractionComponent::StartInteraction()
{
	if (!NearestInteractable.IsValid()
		|| !FocusedInteractables.Contains(NearestInteractable))
	{
		UE_LOG(LogTemp, Warning, TEXT("[StartInteraction]  NearestInteractable이 유효하지 않음"));
		return;
	}
	if (!NearestInteractable.ToWeakInterface()->Execute_CanInteract(NearestInteractable.GetRawObject()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[StartInteraction]  NearestInteractable이 상호작용 가능한 상태가 아님"));
		return;
	}
	FNAInteractionData NearestInteractableData = FocusedInteractables[NearestInteractable];
	if (NearestInteractableData.InteractorActor != GetOwner())
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("[StartInteraction]  NearestInteractable이 대상으로 하는 interactor(%s)가 Owner(%s)와 다름.")
		       , NearestInteractableData.InteractorActor.IsValid()
					? *NearestInteractableData.InteractorActor->GetName()
					: TEXT_NULL
		       , GetOwner()
					? *GetOwner()->GetName()
					: TEXT_NULL);
		return;
	}
	
	bIsInInteraction = true;
	if (NearestInteractable.ToWeakInterface()->HasInteractionDelay())
	{
		float InteractionDelay = NearestInteractable.ToWeakInterface()->GetInteractionDelay();
		FWeakInteractableHandle CapturedNearestInteractable = NearestInteractable;
		if (InteractionDelay > 0.f)
		{
			// @TODO: 상호작용 트리거 딜레이 동안 ux 어떻게 처리할지 고민하기
			GetWorld()->GetTimerManager().ClearTimer(InteractionTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(
				InteractionTimerHandle,
				FTimerDelegate::CreateLambda(
					[this, CapturedNearestInteractable]()
					{
						FNAInteractionResult Result = TryInitiateInteraction(CapturedNearestInteractable);
						//...
						bIsInInteraction = false;
					}),
					InteractionDelay,
					false);
		}
	}
	else
	{
		FNAInteractionResult Result = TryInitiateInteraction(NearestInteractable);
		//...
		bIsInInteraction = false;
	}
}

FNAInteractionResult UNAInteractionComponent::TryInitiateInteraction(FWeakInteractableHandle TargetInteractable)
{
	if (!TargetInteractable.IsValid()
		|| !FocusedInteractables.Contains(TargetInteractable))
	{
		return FNAInteractionResult::InteractionFailed(ENAInteractionFailureReason::IxFR_InvalidTarget,
			FText::FromString(TEXT(
				"Interaction failed: Target interactable is invalid or not currently focused.")));
	}

	if (!TargetInteractable.ToWeakInterface()->Execute_CanInteract(TargetInteractable.GetRawObject()))
	{
		return FNAInteractionResult::InteractionFailed(ENAInteractionFailureReason::IxFR_InvalidTarget,
			FText::FromString(TEXT(
				"Interaction failed: Target interactable is not ready to interact.")));
	}

	FNAInteractionData TargetInteractableData = FocusedInteractables[TargetInteractable];
	if (TargetInteractableData.InteractorActor != GetOwner())
	{
		return FNAInteractionResult::InteractionFailed(ENAInteractionFailureReason::IxFR_InvalidTarget,
		FText::Format(
			FText::FromString(TEXT(
				"Interaction target mismatch: Interactor('{0}') recorded by target differs from this owner('{1}')."))
			, TargetInteractableData.InteractorActor.IsValid()
				  ? *TargetInteractableData.InteractorActor->GetName()
				  : TEXT_NULL
			, GetOwner()
				  ? *GetOwner()->GetName()
				  : TEXT_NULL));
	}
	
	if (ANAPickableItemActor* PickableInteractable =
		Cast<ANAPickableItemActor>(TargetInteractableData.InteractableItemActor.Get()))
	{
		const EPickupMode PickupMode = PickableInteractable->GetPickupMode();
		if (PickupMode == EPickupMode::PM_None)
		{
			return FNAInteractionResult::InteractionFailed(ENAInteractionFailureReason::IxFR_InvalidTarget,
			FText::FromString(TEXT(
				"Interaction failed: Target interactable's pickup mode is invalid.")));
		}
		if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_AutoUse))
		{
			
		}
		if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_Inventory))
		{
			int32 AddResult = TryAddItemToInventory(PickableInteractable);
			if (AddResult == 0)
			{
				// 전부 추가 완료했으므로, 아이템 액터 파괴
			}
			else if (AddResult > 0)
			{
				// 남은 수량이 있으므로, 아이템 액터 보존
			}
			else
			{
				// 전부 추가 실패
				
			}
		}
		if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_CarryOnly))
		{
			
		}
	}
	else if (ANAPlaceableItemActor* PlaceableInteractable =
		Cast<ANAPlaceableItemActor>(TargetInteractableData.InteractableItemActor.Get()))
	{
		
	}
	
}

void UNAInteractionComponent::StopInteraction()
{
	//if (!ensure(bHasPendingUseItem && ActiveInteractable.IsValid())) return;

	ActiveInteractable.ToScriptInterface()->Execute_EndInteract(ActiveInteractable.GetRawObject(), GetOwner());
	
	// 어태치된 액터가 있었다면..??? -> 이건 Interactable에서 처리하기
}

void UNAInteractionComponent::SetActiveInteractable(TScriptInterface<INAInteractableInterface> InteractableActor)
{
	if (ActiveInteractable != InteractableActor)
	{
		ActiveInteractable = InteractableActor;
	}
}

void UNAInteractionComponent::OnInteractionEnded(TScriptInterface<INAInteractableInterface> InteractableActor)
{
	if (!InteractableActor) return;
	// if (bHasPendingUseItem && ActiveInteractable.ToScriptInterface() != InteractableActor)
	// {
	// 	UE_LOG(LogTemp, Warning,
	// 		TEXT("[UNAInteractionComponent::OnInteractionEnded]  상호작용 중인 Interactable(%s)과 상호작용을 멈추려던 Interactable(%s)이 서로 다른 객체였음")
	// 		, *ActiveInteractable.GetRawObject()->GetName(), *InteractableActor.GetObject()->GetName());
	// 	return;
	// }
	// if (!bHasPendingUseItem && NearestInteractable.ToScriptInterface() != InteractableActor)
	// {
	// 	UE_LOG(LogTemp, Warning,
	// 		TEXT("[UNAInteractionComponent::OnInteractionEnded]  상호작용 중인 Interactable(%s)과 상호작용을 멈추려던 Interactable(%s)이 서로 다른 객체였음")
	// 		, *ActiveInteractable.GetRawObject()->GetName(), *InteractableActor.GetObject()->GetName());
	// 	return;
	// }

	ActiveInteractable = nullptr;
	bIsInInteraction = false;
}

int32 UNAInteractionComponent::TryAddItemToInventory(ANAItemActor* ItemActor)
{
	UNAInventoryComponent* InventoryComp = GetOwner()->GetComponentByClass<UNAInventoryComponent>();
	if (!ensureAlways(InventoryComp)) return -1;
	
	if (!ItemActor) return -1;
	UNAItemData* Item = ItemActor->GetItemData();
	if (!Item) return -1;
	
	int32 RemainQuantity =  InventoryComp->TryAddItem(Item);
	if (RemainQuantity == 0) // 전부 추가 성공
	{
		UE_LOG(LogTemp, Warning, TEXT("[TryAddItemToInventory]  (%s) 전부 추가 성공"), *ItemActor->GetName());
	}
	if (RemainQuantity > 0) // 부분 추가
	{
		// 아이템 액터 자체는 잔존하나, 아이템 데이터가 복제됨
		// 해당 아이템 액터에 남아있는 아이템 데이터 / 인벤토리에 적재된 아이템 데이터
		UE_LOG(LogTemp, Warning, TEXT("[TryAddItemToInventory]  (%s) 부분 추가: 남은 수량 (%d)"),
			*ItemActor->GetName(), RemainQuantity);
	}
	else if (RemainQuantity == -1) // 전부 실패
	{
		// 아이템 액터 잔존 및 아이템 데이터 변경 없음
		UE_LOG(LogTemp, Warning, TEXT("[TryAddItemToInventory]  (%s) 전부 추가 실패"), *ItemActor->GetName());
	}
	return RemainQuantity;
}
