// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/NAInteractionComponent.h"

#include "NACharacter.h"
#include "Item/ItemActor/NAItemActor.h"


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

	if (bUpdateInteractionData)
	{
		UpdateInteractionData();
	}
}

AActor* UNAInteractionComponent::TryAttachItemMeshToOwner(INAInteractableInterface* InteractableActor)
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
			if ( HandActor->GetRightHandChildActorComponent() )
			{
				HandActor->GetRightHandChildActorComponent()->SetChildActorClass( ActiveInteractableInstance->GetClass() );
				return HandActor->GetRightHandChildActorComponent()->GetChildActor();
			}
			
			if ( HandActor->GetLeftHandChildActorComponent() )
			{
				HandActor->GetLeftHandChildActorComponent()->SetChildActorClass( ActiveInteractableInstance->GetClass() );
				return HandActor->GetLeftHandChildActorComponent()->GetChildActor();
			}
		}
	}

	return nullptr;
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

bool UNAInteractionComponent::OnInteractableFound(INAInteractableInterface* InteractableActor)
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
		TWeakInterfacePtr<INAInteractableInterface> WeakInteractable(InteractableActor);
		UObject* InteractableObj = WeakInteractable.GetObject();
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

bool UNAInteractionComponent::OnInteractableLost(INAInteractableInterface* InteractableActor)
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

void UNAInteractionComponent::BeginInteraction(/*INAInteractableInterface* InteractableActor*/)
{
	if (!NearestInteractable.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UNAInteractionComponent::BeginInteraction]  NearestInteractable이 유효하지 않음"));
		return;
	}

	ActiveInteractable = NearestInteractable;
	ActiveInteractable.ToWeakInterface()->Execute_BeginInteract(ActiveInteractable.GetRawObject(), GetOwner());
}

void UNAInteractionComponent::EndInteraction(/*INAInteractableInterface* InteractableActor*/)
{
	if (!NearestInteractable.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UNAInteractionComponent::EndInteraction]  NearestInteractable이 유효하지 않음"));
		return;
	}

	ActiveInteractable.ToWeakInterface()->Execute_EndInteract(ActiveInteractable.GetRawObject(), GetOwner());
	ActiveInteractable = nullptr;
}

void UNAInteractionComponent::ExecuteInteraction(/*INAInteractableInterface* InteractableActor*/)
{
	if (!ActiveInteractable.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UNAInteractionComponent::ExecuteInteraction]  ActiveInteractable 유효하지 않음"));
		return;
	}

	ActiveInteractable.ToWeakInterface()->Execute_ExecuteInteract(ActiveInteractable.GetRawObject(), GetOwner());
}

