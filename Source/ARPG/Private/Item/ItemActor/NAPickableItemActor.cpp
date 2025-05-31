#include "Item/ItemActor/NAPickableItemActor.h"

#include "Interaction/NAInteractionComponent.h"

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

// void ANAPickableItemActor::SetPickupMode(EPickupMode InPickupMode)
// {
// }
void ANAPickableItemActor::BeginInteract_Implementation(AActor* Interactor)
{
	Super::BeginInteract_Implementation(Interactor);
	bool bAttached = false;
	bool bMovedToInventory = false;

	if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_Inventory))
	{
		if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_Holdable))
		{
			if (UNAInteractionComponent* InteractComp = TryGetInteractionComponent(Interactor))
			{
				if ( const AActor* NewItemActor = InteractComp->TryAttachItemMeshToOwner( this );
					 !NewItemActor )
				{
					// 손이 꽉 찼거나 컴포넌트에서 알수없는 예외가 발생한 경우
					bMovedToInventory = true;
				}
				else
				{
					// 이때 새로운 Item Actor가 Child Actor Component에 의해 스폰됨
					bAttached = true;	
				}
			}
		}
		else
		{
			// 인벤토리로 이동
			bMovedToInventory = true;
		}
	}

	if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_CarryOnly))
	{
		if (UNAInteractionComponent* InteractComp = TryGetInteractionComponent(Interactor))
		{
			// 일단 붙이기는 해야함, 애니메이션 실행 등의 것들이 있기에
			// 이때 새로운 Item Actor가 Child Actor Component에 의해 스폰됨
			if ( ANAItemActor* NewItemActor = Cast<ANAItemActor>( InteractComp->TryAttachItemMeshToOwner( this ) ) )
			{
				if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_AutoUse))
				{
					// TODO: Child Actor의 Interaction을 바로 수행해야 함
					// BeginInteraction을 타고 실행되는게 아니라서... Component를 우회하고 강제 실행?
					NewItemActor->ExecuteInteract_Implementation( Interactor );
				}

				bAttached = true;
			}
			else if ( !NewItemActor )
			{
				// 들어야하는 아이템인데 손이 바쁘니까 취소
				EndInteract_Implementation( Interactor );
			}
		}
	}

	// 하나만 해라
	check ( bAttached ^ bMovedToInventory );

	if ( bAttached || bMovedToInventory )
	{
		// - ChildActorComponent가 생성하는 Actor로 이전
		// - 인벤토리에 아이템 정보를 저장하고 아이템을 파괴
		Destroy(); 
	}
}

void ANAPickableItemActor::EndInteract_Implementation(AActor* Interactor)
{
	Super::EndInteract_Implementation(Interactor);
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
}

void ANAPickableItemActor::ExecuteInteract_Implementation(AActor* Interactor)
{
	Super::ExecuteInteract_Implementation(Interactor);
	// 아이템 효과 적용, 구현부는 자식에게?
}