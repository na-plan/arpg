#include "Item/ItemActor/NAPickableItemActor.h"

#include "Interaction/NAInteractionComponent.h"

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

	if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_Inventory))
	{
		if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_Holdable))
		{

		}
		else
		{
		}
	}

	if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_CarryOnly))
	{
		// @TODO: 메쉬 붙이기
		if (UNAInteractionComponent* InteractComp = TryGetInteractionComponent(Interactor))
		{
			InteractComp->AttachItemMeshToOwner(this);
			if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_AutoUse))
			{
				// @TODO: 줍자마자 자동 사용
			}
			else
			{

			}
		}
	}
}

void ANAPickableItemActor::EndInteract_Implementation(AActor* Interactor)
{
	Super::EndInteract_Implementation(Interactor);
}

void ANAPickableItemActor::ExecuteInteract_Implementation(AActor* Interactor)
{
	Super::ExecuteInteract_Implementation(Interactor);

	// @TODO: PickupMode에 따라 상호작용 다르게

	if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_Inventory))
	{
		if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_Holdable))
		{

		}
		else
		{
			// @TODO: 인벤토리에 넘기기
		}
	}

	if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_CarryOnly))
	{
		if (UNAInteractionComponent* InteractComp = TryGetInteractionComponent(Interactor))
		{
			if (EnumHasAnyFlags(PickupMode, EPickupMode::PM_AutoUse))
			{
				// @TODO: 줍자마자 자동 사용
			}
			else
			{

			}
		}
	}
}