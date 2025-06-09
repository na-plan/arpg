#include "Item/ItemActor/NAPickableItemActor.h"

#include "Components/ShapeComponent.h"
#include "Interaction/NAInteractionComponent.h"
#include "UObject/FastReferenceCollector.h"

ANAPickableItemActor::ANAPickableItemActor(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void ANAPickableItemActor::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
}

void ANAPickableItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ANAPickableItemActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ANAPickableItemActor::BeginPlay()
{
	Super::BeginPlay();
}

int32 ANAPickableItemActor::TryPerformAutoUse(AActor* User)
{
	if (!EnumHasAnyFlags(PickupMode, EPickupMode::PM_AutoUse)) return 0;
	
	const int32 ConsumedQty = PerformAutoUse_Impl(User);
	if (ConsumedQty > 0)
	{
		if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_Inventory))
		{
			int32 CurrQty = GetItemData()->GetQuantity();
			if (CurrQty - ConsumedQty <= 0)
			{
				// 자동 사용 후 아이템 다 씀 -> 이 액터 인스턴스 & 아이템 데이터 제거
				if (UNAItemEngineSubsystem::Get()->DestroyRuntimeItemData(GetItemData()))
				{
					Destroy();
					return -1;
				}
			}
			else
			{
				GetItemData()->SetQuantity(CurrQty - ConsumedQty);
			}
		}
	}
	return ConsumedQty;
}

int32 ANAPickableItemActor::PerformAutoUse_Impl(AActor* User)
{
	return 0;
}

// void ANAPickableItemActor::SetPickupMode(EPickupMode InPickupMode)
// {
// }

// "일반적으로 대부분의 상황에서", InteractionComponent::BeginInteraction에 의해 호출됨(될 것을 전제)
// 즉, 이 함수가 호출될 때, 이미 InteractionComponent 쪽에서 상호작용할 아이템과 상호작용 process이 시작됨을 알고 있다는 것이 전제됨
// BeginInteract: 일단 상호작용 실행 후 결과에 따라 분기 후처리 (실제 상호작용(아이템 인번토리 이동, 어태치 등등)은 ExecuteInteract에서)											  
void ANAPickableItemActor::BeginInteract_Implementation(AActor* Interactor)
{
	Super::BeginInteract_Implementation(Interactor);
	
	UNAInteractionComponent* InteractComp = TryGetInteractionComponent(Interactor);
	if (!ensureAlways(IsValid(InteractComp))) return;

	// 일단 상호작용 실행부터 시도
	const bool bSucceed = Execute_ExecuteInteract(this, Interactor);

	if (bSucceed) // 인벤토리에 이동 성공 및 (필요했다면) 어태치 성공
	{
		if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_Inventory))
		{
			if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_Holdable))
			{
				// PM_Holdable -> 어태치도 성공했음. 상호작용 종료가 아니라 사용 대기 상태로
				return;
			}

			// 인벤토리에 성공적으로 (남은 수량 없음) 추가되었다면 상호작용 종료 & 이 액터 인스턴스 제거
			Execute_EndInteract(this, Interactor);
			Destroy();
			return;
		}
		
		if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_CarryOnly))
		{
			if (bSucceed)
			{
				// 어태치 성공 후 사용 대기 상태로
				return;
			}
		}
	}
	
	// 인벤토리에 못넣었거나(or 부분 추가 후 남는 수량 있음), CarryOnly인데 어태치에 실패했거나
	Execute_EndInteract(this, Interactor);
}

// 들기가 가능한 아이템의 경우:
// 1. 인벤토리에 적재 가능한 아이템: 먼저 인벤토리에 Add Item을 시도한 뒤, add에 성공하면 attach 시도
//								어태치에 성공하면 아이템 사용 대기 상태에 진입		
// 2. 인벤토리에 적재 불가능한 아이템: 바로 어태치 시도 후 성공 여부에 따라 아이템 사용 대기 상태에 진입
// @ return	상호작용 실행 성공 여부 -> false면 BeginInteract에서 EndInteract 호출함
bool ANAPickableItemActor::ExecuteInteract_Implementation(AActor* Interactor)
{
	Super::ExecuteInteract_Implementation(Interactor);
	// 아이템 효과 적용, 구현부는 자식에게?
	UNAInteractionComponent* InteractComp = TryGetInteractionComponent(Interactor);
	if (!ensureAlways(IsValid(InteractComp))) return false;

	if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_AutoUse))
	{
		int32 bResult = TryPerformAutoUse(Interactor);
		if (bResult == -1) return true;
		
		// 자동 사용에 실패하거나, 자동 사용 후 남은 수량이 있다면 -> 인벤토리에 add 시도
	}

	// 인벤토리에 적재 가능한 아이템이면 -> 인벤토리에 add부터 시도
	// @TODO : Holdable인데 부분 추가된 경우: 부분 추가된 것들에 한정하여 어태치 시도 ?
	// @TODO : (근데 보통 Holdable 가능한 아이템이면 1 액터 1 수량이라 자주 있는 시나리오는 아님)
	if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_Inventory))
	{
		const bool bSucceed = InteractComp->TryAddItemToInventory(this);
		
		if (bSucceed) // 인벤토리에 전부 추가 성공
		{
			if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_Holdable))
			{
				TScriptInterface<INAInteractableInterface> NewInteractableActor = InteractComp->
					TryAttachItemMeshToOwner(this);
				if (NewInteractableActor != nullptr)
				{
					// @TODO: 사용 대기 상태(bIsOnInteract이 true)일 때 ... 여기서 수행해야할 뭔가가 있을지 고민해보기

					NewInteractableActor->DisableOverlapDuringInteraction();
					return true;
				}
			}
			else // holdable이 아니면서, 전부 추가 성공 -> 상호장용 종료 true
			{
				return true;
			}
		}
		// 부분 추가 or 추가 실패 -> (임시) 부분 추가되면 어태치 시도 안함
	}
	// 바로 어태치 시도
	else if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_CarryOnly))
	{
		TScriptInterface<INAInteractableInterface> NewInteractableActor = InteractComp->TryAttachItemMeshToOwner(this);
		if (NewInteractableActor != nullptr)
		{
			// @TODO: 사용 대기 상태(bIsOnInteract이 true)일 때 ... 여기서 수행해야할 뭔가가 있을지 고민해보기

			NewInteractableActor->DisableOverlapDuringInteraction();
			return true;
		}
	}

	// 상호작용 종료
	return false;
}

void ANAPickableItemActor::EndInteract_Implementation(AActor* Interactor)
{
	Super::EndInteract_Implementation(Interactor);
	
	UNAInteractionComponent* InteractComp = TryGetInteractionComponent(Interactor);
	if (!ensureAlways(IsValid(InteractComp))) return;
	
	// 아이템 드랍?
	// 아이템이 부착이 되어 있는 경우
	if ( GetParentActor() == Interactor )
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		SpawnParams.Owner = nullptr;
		SpawnParams.Template = this;
		SpawnParams.bNoFail = true;
	
		const FVector SpawnLocation = GetActorLocation();
		const FRotator SpawnRotation = FRotator::ZeroRotator;
		const AActor* Spawned = GetWorld()->SpawnActor( GetClass(), &SpawnLocation, &SpawnRotation, SpawnParams );
		check( Spawned );
	
		// ChildActorComponent에 의해 만들어진 액터인 경우
		// 부착되어 있던 아이템을 파괴 ( = 자신을 파괴 )
		if ( UChildActorComponent* ChildActorComponent = Cast<UChildActorComponent>( GetParentComponent() ) )
		{
			ChildActorComponent->DestroyChildActor();
			ChildActorComponent->SetChildActorClass( nullptr );
		}
		else
		{
			// 아니면 새로 스폰된 아이템에게 유산을 넘겨줌
			Destroy();
		}
	}

	if (!IsPendingKillPending())
	{
		ItemRootShape->SetGenerateOverlapEvents(true);
	}
}
