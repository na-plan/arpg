// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/NAInventoryComponent.h"

#include "Inventory/GameInstance/NAInventoryGameInstanceSubsystem.h"
#include "Item/ItemActor/NAItemActor.h"
#include "Inventory/Widget/NAInventoryWidget.h"


UNAInventoryGameInstanceSubsystem* GetInvSubsys(const UNAInventoryComponent* InvComp)
{
	UNAInventoryGameInstanceSubsystem* InvSubsys = nullptr;
	if (InvComp)
	{
		if (UWorld* World = InvComp->GetWorld()) {
			if (UGameInstance* GI = World->GetGameInstance()) {
				InvSubsys = GI->GetSubsystem<UNAInventoryGameInstanceSubsystem>();
			}
		}
	}
	return InvSubsys;
}

// Sets default values for this component's properties
UNAInventoryComponent::UNAInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FClassFinder<UNAInventoryWidget> InventoryWidgetClass(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/00_ProjectNA/Inventory/BP_NAInventoryWidget.BP_NAInventoryWidget_C'"));
	check(InventoryWidgetClass.Class);
	SetWidgetClass(InventoryWidgetClass.Class);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetEnableGravity(false);
	CanCharacterStepUpOn = ECB_No;
	SetRelativeRotation(FRotator(-10.0f, 0.0f, 0.0f));
	SetWidgetSpace(EWidgetSpace::World);
	SetDrawSize(FVector2D(1280, 720));
	SetGeometryMode(EWidgetGeometryMode::Cylinder);
	SetCylinderArcAngle(180.f);
	bIsTwoSided = true;
	SetBlendMode(EWidgetBlendMode::Masked);
	OpacityFromTexture = 0.8f;
	SetVisibility(false);
	SetWindowVisibility(EWindowVisibility::SelfHitTestInvisible);
}


// Called when the game starts
void UNAInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UNAInventoryComponent::RemoveSingleInstanceOfItem(UNAItemData* ItemToRemove)
{
	InventoryContents.RemoveSingle(ItemToRemove);
	//OnInventoryUpdated.Broadcast();
}

int32 UNAInventoryComponent::RemoveAmountOfItem(UNAItemData* ItemToRemove, int32 DesiredAmountToRemove)
{
	const int32 ActualAmountToRemove = FMath::Min(DesiredAmountToRemove, ItemToRemove->Quantity);
	ItemToRemove->SetQuantity(ItemToRemove->Quantity - ActualAmountToRemove);
	InventoryTotalWeight -= ActualAmountToRemove * ItemToRemove->GetItemSingleWeight();

	//OnInventoryUpdated.Broadcast();
	return ActualAmountToRemove;
}

void UNAInventoryComponent::SpiltExistingStack(UNAItemData* ItemToSplit, const int32 AmountToSplit)
{
	if (!(InventoryContents.Num() + 1 > InventorySlotsCapacity))
	{
		RemoveAmountOfItem(ItemToSplit, AmountToSplit);
		HandleAddNewItem(ItemToSplit, AmountToSplit);
	}
}

FItemAddResult UNAInventoryComponent::HandleNonStackableItems(UNAItemData* InItem, int32 RequestedAddAmount)
{
	return {};
}

int32 UNAInventoryComponent::HandleStackableItems(UNAItemData* InItem, int32 RequestedAddAmount)
{
	return 0;
}

int32 UNAInventoryComponent::CalculateWeightAddAmount(UNAItemData* InItem, int32 RequestedAddAmount)
{
	const int32 WeightMaxAddAmount = FMath::FloorToInt((GetWeightCapacity() - InventoryTotalWeight) / InItem->GetItemSingleWeight());
	if (WeightMaxAddAmount >= RequestedAddAmount)
	{
		return RequestedAddAmount;
	}
	return WeightMaxAddAmount;
}

int32 UNAInventoryComponent::CalculateNumberForFullStack(UNAItemData* StackableItem, int32 InitialRequestedAddAmount)
{
	int32 Result = -1;
	if (const FNAItemBaseTableRow* StackableItemData = StackableItem->GetItemMetaDataStruct<FNAItemBaseTableRow>())
	{
		const int32 AddAmountToMakeFullStack = StackableItemData->NumericData.MaxInventoryStackSize - StackableItem->Quantity;
		Result = FMath::Min(InitialRequestedAddAmount, AddAmountToMakeFullStack);
	}
	return Result;
}

// Called every frame
void UNAInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

UNAItemData* UNAInventoryComponent::FindMatchingItem(UNAItemData* InItem) const
{
	if (InItem)
	{
		if (InventoryContents.Contains(InItem))
		{
			return InItem;
		}
	}
	return nullptr;
}

UNAItemData* UNAInventoryComponent::FindNextItemByID(UNAItemData* InItem) const
{
	if (InItem)
	{
		if (const TArray<UNAItemData*>::ElementType* Result = InventoryContents.FindByKey(InItem))
		{
			return *Result;
		}
	}
	return nullptr;
}

UNAItemData* UNAInventoryComponent::FindNextPartialStack(UNAItemData* InItem) const
{
	if (InItem)
	{
		if (const TArray<UNAItemData*>::ElementType* Result =
			InventoryContents.FindByPredicate([&InItem](const UNAItemData* InventoryItem)
			{
				return InventoryItem->GetItemID() == InItem->GetItemID() && !InventoryItem->IsFullItemStack();
			}))
		{
			return *Result;
		}
	}
	return nullptr;
}

void UNAInventoryComponent::HandleAddNewItem(UNAItemData* Item, const int32 AmountToAdd)
{
	UNAInventoryGameInstanceSubsystem* InveSubsys = GetInvSubsys(this);
	if (InveSubsys)
	{
		InveSubsys->AddItemToInventory(Item->GetItemID(), Item->Quantity);
	}
}

FItemAddResult UNAInventoryComponent::HandleAddItem(UNAItemData* ItemToAdd, int32 RequestedAddAmount)
{
	return FItemAddResult();
}

void UNAInventoryComponent::HandleRemoveSingleItemActor(UNAItemData* InItem)
{
	UNAInventoryGameInstanceSubsystem* InveSubsys = GetInvSubsys(this);
	if (InveSubsys)
	{
		InveSubsys->RemoveItemFromInventory(InItem->GetItemID(), 1);
	}
}

int32 UNAInventoryComponent::HandleRemoveAmountOfItemActors(UNAItemData* InItem, int32 DesiredAmountToRemove)
{
	UNAInventoryGameInstanceSubsystem* InveSubsys = GetInvSubsys(this);
	int32 RemovedAmount = 0;
	if (InveSubsys)
	{
		RemovedAmount = InveSubsys->RemoveItemFromInventory(InItem->GetItemID(), DesiredAmountToRemove);
	}
	return RemovedAmount;
}

void UNAInventoryComponent::SplitExistingStack(UNAItemData* InItem, const int32 AmountToSplit)
{
}

void UNAInventoryComponent::ReleaseInventoryWidget()
{
	if (UUserWidget* InventoryWidget = GetWidget())
	{
		SetVisibility(true);
		InventoryWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UNAInventoryComponent::CollapseInventoryWidget()
{
	if (UUserWidget* InventoryWidget = GetWidget())
	{
		SetVisibility(false);
		InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

