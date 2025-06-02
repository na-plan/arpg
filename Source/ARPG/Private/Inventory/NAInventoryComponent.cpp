// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/NAInventoryComponent.h"

#include "Inventory/GameInstance/NAInventoryGameInstanceSubsystem.h"
#include "Item/EngineSubsystem/NAItemEngineSubsystem.h"
#include "Item/ItemActor/NAItemActor.h"
#include "Inventory/Widget/NAInventoryWidget.h"

// 슬롯 ID에서 문자열 추출 헬퍼 함수
static int32 ExtractSlotNumber(const FName& SlotID)
{
	const FString SlotStr = SlotID.ToString();
	int32 UnderscoreIndex = INDEX_NONE;
	if (SlotStr.FindLastChar(TEXT('_'), UnderscoreIndex))
	{
		const FString NumberStr = SlotStr.Mid(UnderscoreIndex + 1);
		return FCString::Atoi(*NumberStr);
	}
	return INT_MAX;
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

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		// 인벤토리 슬롯 초기화
		InventoryContents.Reserve(MaxTotalSlots);
	
		// 인벤토리 슬롯 25개: 1~25까지 Inven_nn 슬롯 키를 채우고, 값은 nullptr (TWeakObjectPtr이므로 nullptr 가능)
		for (int32 i = 1; i <= MaxInventorySlots; ++i)
		{
			FString SlotNameStr = FString::Printf(TEXT("Inven_%02d"), i);
			InventoryContents.Add(FName(*SlotNameStr), nullptr);
		}
	
		// 무기 슬롯 4개: 1~4까지 Weapon_nn 슬롯 키를 채우고, 값은 nullptr
		for (int32 i = 1; i <= MaxWeaponSlots; ++i)
		{
			FString SlotNameStr = FString::Printf(TEXT("Weapon_%02d"), i);
			InventoryContents.Add(FName(*SlotNameStr), nullptr);
		}
	}
}


// Called when the game starts
void UNAInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	SetWindowVisibility(EWindowVisibility::SelfHitTestInvisible);
	
}

void UNAInventoryComponent::HandleRemoveSingleItemData(UNAItemData* ItemDataToRemove)
{
	//InventoryContents.RemoveSingle(ItemToRemove);
	//OnInventoryUpdated.Broadcast();

	if (!IsValid(ItemDataToRemove))
	{
		ensure(false);
		return;
	}
	
	if (UNAInventoryGameInstanceSubsystem* InvenSubsys = UNAInventoryGameInstanceSubsystem::Get(GetWorld()))
	{
		InvenSubsys->RemoveItemFromInventory(ItemDataToRemove->GetItemID(), );
	}
}

int32 UNAInventoryComponent::HandleRemoveAmountOfItems(UNAItemData* ItemToRemove, int32 DesiredAmountToRemove)
{
	const int32 ActualAmountToRemove = FMath::Min(DesiredAmountToRemove, ItemToRemove->Quantity);
	ItemToRemove->SetQuantity(ItemToRemove->Quantity - ActualAmountToRemove);
	InventoryTotalWeight -= ActualAmountToRemove * ItemToRemove->GetItemSingleWeight();

	//OnInventoryUpdated.Broadcast();
	return ActualAmountToRemove;
}

void UNAInventoryComponent::SortInventoryItems()
{
	
}

void UNAInventoryComponent::SplitExistingStack(UNAItemData* ItemToSplit, const int32 AmountToSplit)
{
	if (InventoryContents.Num() + 1 <= InventorySlotsCapacity)
	{
		HandleRemoveAmountOfItems(ItemToSplit, AmountToSplit);
		HandleAddNewItem(ItemToSplit, AmountToSplit);
	}
}

TArray<UNAItemData*> UNAInventoryComponent::GetInventoryContents() const
{
	TArray<UNAItemData*> InventoryContentsArray;
	for (const auto& Pair : InventoryContents)
	{
		if (Pair.Value.IsValid())
		{
			InventoryContentsArray.Add(Pair.Value.Get());
		}
	}
	return InventoryContentsArray;
}

FNAItemAddResult UNAInventoryComponent::AddNonStackableItem_Internal(UNAItemData* InputItem, const TArray<FName>& InEmptySlots)
{
	if (InputItem && InputItem->GetItemMaxSlotStackSize() == 1 && InputItem->GetMaxInventoryHoldCount() >= 0)
	{
		if (const FNAItemBaseTableRow* InputItemData = InputItem->GetItemMetaDataStruct())
		{
			if (InEmptySlots.IsEmpty())
			{
				return FNAItemAddResult::AddedNone(FText::Format(
						FText::FromString("Could not add {0} to the inventory. All inventory slots are full."),
						InputItemData->TextData.Name));
			}
		
			// check if in the input item has a valid quantity (non-stackable item's quantity must be 1)
			if (InputItem->GetQuantity() != 1)
			{
				ensure(false);
				return FNAItemAddResult::AddedNone(FText::Format(
					FText::FromString("Could not add {0} to the inventory. Item has a invalid quantity value."),
					InputItemData->TextData.Name));
			}

			// check MaxInventoryHoldCount
			int32 MaxInventoryHoldCount = InputItem->GetMaxInventoryHoldCount();
			if (MaxInventoryHoldCount == 1)
			{
				UClass* InputItemActorClass = InputItem->GetItemActorClass();
				if (HasSameItemClass(InputItemActorClass))
				{
					return FNAItemAddResult::AddedNone(FText::Format(
					FText::FromString("Could not add {0} to the inventory. This item cannot be carried more than once."),
					InputItemData->TextData.Name));
				}
			}
	
			const bool bSucceed = HandleAddNewItem(InputItem, InEmptySlots[0]);
			if (bSucceed)
			{
				return FNAItemAddResult::AddedAll(1, FText::Format(
				   FText::FromString("Successfully added a single {0} to the inventory."), InputItemData->TextData.Name));
			}
		}
	}

	ensureAlwaysMsgf(false, TEXT("[UNAInventoryComponent::HandleNonStackableItems]  Failed to add the non-stackable item to the inventory."));
	return FNAItemAddResult::AddedNone(FText::FromString("Failed to add the non-stackable item to the inventory."));
}

FNAItemAddResult UNAInventoryComponent::AddStackableItem_Internal(UNAItemData* InputItem, const TArray<FName>& InEmptySlots)
{
	if (InputItem && InputItem->GetQuantity() > 0 && InputItem->GetMaxInventoryHoldCount() >= 0)
	{
		if (const FNAItemBaseTableRow* InputItemData = InputItem->GetItemMetaDataStruct())
		{
			int32 RequestedAddAmount = InputItem->GetQuantity();
			int32 MaxInventoryHoldCount = InputItem->GetMaxInventoryHoldCount();
			int32 MaxSlotStackSize = InputItem->GetItemMaxSlotStackSize();
			int32 RemainingHoldCount = 0;
			int32 AmountToDistribute = 0;
			int32 ActualAddAmount = 0;
			
			TArray<FName> AddableSlots;
			UClass* InputItemActorClass = InputItem->GetItemActorClass();
			const bool bHasSameItemSlots = HasSameItemClass(InputItemActorClass);
			const bool bHasEmptySlots = !InEmptySlots.IsEmpty();
			if (!bHasSameItemSlots && !bHasEmptySlots)
			{
				// 추가 가능한 슬롯 없음
				return FNAItemAddResult::AddedNone(FText::FromString("Failed to add the stackable item to the inventory."));
			}
			
			if (bHasSameItemSlots)
			{
				int32 CurrentTotalHoldCount = 0;
				for (const FName& Slot : AddableSlots)
				{
					if (!InventoryContents.Contains(Slot))
					{
						ensure(false);
						continue;
					}
					CurrentTotalHoldCount += InventoryContents[Slot]->GetQuantity();
				}

				// MaxInventoryHoldCount == 0 이면 소지 상한 없음
				RemainingHoldCount = MaxInventoryHoldCount == 0 ? RequestedAddAmount : MaxInventoryHoldCount - CurrentTotalHoldCount;
				if (RemainingHoldCount <= 0)
				{
					// 인벤토리 소지 상한 초과
					return FNAItemAddResult::AddedNone(FText::FromString("Failed to add the stackable item to the inventory."));
				}

				AmountToDistribute =  FMath::Min(RequestedAddAmount, RemainingHoldCount);
				
				for (const FName& Slot : AddableSlots)
				{
					if (!InventoryContents.Contains(Slot))
					{
						ensure(false);
						continue;
					}
		
					int32 ExtraStackAmount = CalculateNumberForFullSlotStack(Slot);
					if (ExtraStackAmount <= 0)
					{
						continue;
					}
					int32 StackToAdd = FMath::Min(ExtraStackAmount, AmountToDistribute);
					if (StackToAdd > 0)
					{
						int32 NewQuantity = InputItem->GetQuantity() + StackToAdd;
						InventoryContents[Slot]->SetQuantity(NewQuantity);
						AmountToDistribute -= StackToAdd;
						ActualAddAmount += StackToAdd;
					}
					
					if (AmountToDistribute == 0)
					{
						// 새 슬롯에 아이템 추가하지 않고 add item process 종료, 모든 아이템 추가 성공
						if (RequestedAddAmount == ActualAddAmount)
						{
							return FNAItemAddResult::AddedAll(ActualAddAmount, FText::Format(
						   FText::FromString("Successfully added all of {0} to the inventory."),InputItemData->TextData.Name));
						}

						// 같은 아이템이 있던 슬롯에 새로운 아이템을 채워넣는 도중 상한 선에 걸림, 부분 추가만 성공
						return FNAItemAddResult::AddedPartial(ActualAddAmount, FText::Format(
							FText::FromString("Added partial amount of {0} to the inventory."),InputItemData->TextData.Name));
					}
				}

				ensure(AmountToDistribute > 0);
				// 같은 아이템이 있던 슬롯에 새로운 아이템을 채워넣고 난 후, 남은 추가 수량이 있음
				if (AmountToDistribute > 0)
				{
					// 빈 슬롯 없음
					if (!bHasEmptySlots)
					{
						if (ActualAddAmount <= 0)
						{
							// 하나도 추가 못함 && 추가 가능한 빈 슬롯 없음
							return FNAItemAddResult::AddedNone(FText::FromString("Failed to add the stackable item to the inventory."));
						}

						// 부분 추가 성공 && 남은 빈 슬롯 없음
						return FNAItemAddResult::AddedPartial(ActualAddAmount, FText::Format(
							FText::FromString("Added partial amount of {0} to the inventory."),InputItemData->TextData.Name));
					}

					// 빈 슬롯 있고, 인벤토리 소지 상한 없음 (무한) || RequestedAddAmount가 인벤토리 소지 상한에 걸리지 않음
					// 빈 슬롯 개수가 모자라면 부분 추가, 안 모자르면 전부 추가 성공
					// 빈 슬롯이 있지만, RequestedAddAmount가 인벤토리 소지 상한에 걸림 - 부분 추가
					for (const FName& Slot : InEmptySlots)
					{
						if (InventoryContents[Slot].IsValid())
						{
							UE_LOG(LogTemp, Warning, TEXT("[::] 왜 안비어있는데"));
							continue;
						}
						int32 StackToAdd = FMath::Min(AmountToDistribute, MaxSlotStackSize);
						if (StackToAdd > 0)
						{
							int32 NewQuantity = InputItem->GetQuantity() + StackToAdd;
							InputItem->SetQuantity(NewQuantity);
							const bool bSucceed = HandleAddNewItem(InputItem, Slot);
							if (bSucceed)
							{
								AmountToDistribute -= StackToAdd;
								ActualAddAmount += StackToAdd;
							}
						}
						if (AmountToDistribute == 0 && RequestedAddAmount == ActualAddAmount)
						{
							return FNAItemAddResult::AddedAll(ActualAddAmount, FText::Format(
							FText::FromString("Successfully added all of {0} to the inventory."),InputItemData->TextData.Name));
						}
					}

					// 추가 해야할 수량이 남았지만, 더 이상 빈 슬롯 없음 or 인벤토리 소지 상한에 걸림
					return FNAItemAddResult::AddedPartial(ActualAddAmount, FText::Format(
							FText::FromString("Added partial amount of {0} to the inventory."),InputItemData->TextData.Name));
				}
			}

			ensure(!bHasSameItemSlots && bHasEmptySlots);
			// 아예 처음 들어온 아이템 추가 && 새 슬롯에 아이템 추가
			for (const FName& Slot : InEmptySlots)
			{
				if (InventoryContents[Slot].IsValid())
				{
					UE_LOG(LogTemp, Warning, TEXT("[::] 왜 안비어있는데"));
					continue;
				}
				int32 StackToAdd = FMath::Min(AmountToDistribute, MaxSlotStackSize);
				if (StackToAdd > 0)
				{
					int32 NewQuantity = InputItem->GetQuantity() + StackToAdd;
					InputItem->SetQuantity(NewQuantity);
					const bool bSucceed = HandleAddNewItem(InputItem, Slot);
					if (bSucceed)
					{
						AmountToDistribute -= StackToAdd;
						ActualAddAmount += StackToAdd;
					}
				}
				if (AmountToDistribute == 0 && RequestedAddAmount == ActualAddAmount)
				{
					// 빈 슬롯 전부 순회하기 전에 아이템 전부 추가 완료
					return FNAItemAddResult::AddedAll(ActualAddAmount, FText::Format(
					FText::FromString("Successfully added all of {0} to the inventory."),InputItemData->TextData.Name));
				}
			}

			// 추가 해야할 수량이 남았지만, 더 이상 빈 슬롯 없음 
			ensure(AmountToDistribute > 0);
			return FNAItemAddResult::AddedPartial(ActualAddAmount, FText::Format(
					FText::FromString("Added partial amount of {0} to the inventory."),InputItemData->TextData.Name));
		}
	}
	
	return FNAItemAddResult::AddedNone(FText::FromString("Failed to add the stackable item to the inventory."));
}

// int32 UNAInventoryComponent::CalculateWeightAddAmount(UNAItemData* InItem, int32 RequestedAddAmount)
// {
// 	const int32 WeightMaxAddAmount = FMath::FloorToInt((GetWeightCapacity() - InventoryTotalWeight) / InItem->GetItemSingleWeight());
// 	if (WeightMaxAddAmount >= RequestedAddAmount)
// 	{
// 		return RequestedAddAmount;
// 	}
// 	return WeightMaxAddAmount;
// }

int32 UNAInventoryComponent::CalculateNumberForFullSlotStack(const FName& SlotID) const
{
	int32 Result = -1;
	if (!IsEmptySlot(SlotID))
	{
		return InventoryContents[SlotID]->GetMaxInventoryHoldCount() - InventoryContents[SlotID]->GetQuantity();
	}
	return Result;
}

// Called every frame
void UNAInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

int32 UNAInventoryComponent::TryAddItem(UNAItemData* ItemToAdd)
{
	if (ItemToAdd)
	{
		if (ItemToAdd->IsPickableItem())
		{
			TArray<FName> EmptySlots;
			int32 RequestedAddAmount = ItemToAdd->GetQuantity();
			FNAItemAddResult Result;
			if (ItemToAdd->IsStackableItem())
			{
				GetEmptyInventorySlotIDs(EmptySlots);
				Result = AddStackableItem_Internal(ItemToAdd, EmptySlots);
				if (Result.OperationResult != ENAItemAddStatus::IAR_NoItemAdded)
				{
					SortInventoryItems();
					return  RequestedAddAmount - Result.ActualAmountAdded;
				}
				return -1;
			}

			if (ItemToAdd->GetItemType() == EItemType::IT_Weapon)
			{
				GetEmptyWeaponSlotIDs(EmptySlots);
			}
			else
			{
				GetEmptyInventorySlotIDs(EmptySlots);
			}
			Result = AddNonStackableItem_Internal(ItemToAdd, EmptySlots);
			if (Result.OperationResult != ENAItemAddStatus::IAR_NoItemAdded)
			{
				SortInventoryItems();
				return RequestedAddAmount - Result.ActualAmountAdded;
			}
			return -1;
		}
		UE_LOG(LogTemp, Warning, TEXT("[UNAInventoryComponent::TryAddItem]  인벤토리에 소지 불가능한 아이템."));
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[UNAInventoryComponent::TryAddItem]  ItemToAdd was null."));
	return -1;
}

bool UNAInventoryComponent::HasSameItemClass(UClass* ClassToCheck, TArray<FName>* OutSlotList) const
{
	if (ClassToCheck == nullptr)
	{
		return false;
	}

	bool bFoundAny = false;
	for (const auto& Pair : InventoryContents)
	{
		if (Pair.Value.IsValid())
		{
			if (ClassToCheck == Pair.Value->GetItemActorClass())
			{
				bFoundAny =  true;
				if (OutSlotList)
				{
					OutSlotList->Add(Pair.Key);
				}
				else
				{
					return bFoundAny;
				}
			}
		}
	}
	return bFoundAny;
}

int32 UNAInventoryComponent::GetRemainingStackCapacityInSlot(const FName& SlotID) const
{
	if (!IsEmptySlot(SlotID))
	{
		const int32 CurrentStackSize = InventoryContents[SlotID]->GetQuantity();
		const int32 MaxStackSize = InventoryContents[SlotID]->GetItemMaxSlotStackSize();
		return MaxStackSize - CurrentStackSize;
	}
	return -1;
}

bool UNAInventoryComponent::IsEmptySlot(const FName& SlotID) const
{
	if (!SlotID.IsNone() && InventoryContents.Contains(SlotID))
	{
		return !InventoryContents[SlotID].IsValid();
	}
	return false;
}

void UNAInventoryComponent::GetEmptyInventorySlotIDs(TArray<FName>& OutEmptySlots) const
{
	OutEmptySlots.Empty();
	OutEmptySlots.Reserve(MaxInventorySlots);

	for (const auto& Pair : InventoryContents)
	{
		// 1) 값이 비어있으면서(FName→값이 nullptr) “Inven_”으로 시작하는 Key만 수집
		const FName& SlotID = Pair.Key;
		if (!Pair.Value.IsValid())
		{
			const FString SlotStr = SlotID.ToString();
			if (SlotStr.StartsWith(TEXT("Inven_")))
			{
				OutEmptySlots.Add(SlotID);
			}
		}
	}

	// 2) 언더바 뒤 숫자 기준으로 오름차순 정렬
	// OutEmptySlots.Sort(
	// 	[](const FName& A, const FName& B)
	// 	{
	// 		return ExtractSlotNumber(A) < ExtractSlotNumber(B);
	// 	}
	// );
}
void UNAInventoryComponent::GetEmptyWeaponSlotIDs(TArray<FName>& OutEmptySlots) const
{
	OutEmptySlots.Empty();
	OutEmptySlots.Reserve(MaxWeaponSlots);

	for (const auto& Pair : InventoryContents)
	{
		// 1) 값이 비어있으면서 “Weapon_”으로 시작하는 Key만 수집
		const FName& SlotID = Pair.Key;
		if (!Pair.Value.IsValid())
		{
			const FString SlotStr = SlotID.ToString();
			if (SlotStr.StartsWith(TEXT("Weapon_")))
			{
				OutEmptySlots.Add(SlotID);
			}
		}
	}

	// 2) 언더바 뒤 숫자 기준으로 오름차순 정렬
	// OutEmptySlots.Sort(
	// 	[](const FName& A, const FName& B)
	// 	{
	// 		return ExtractSlotNumber(A) < ExtractSlotNumber(B);
	// 	}
	// );
}

int32 UNAInventoryComponent::GetCurrentTotalHoldCount(UClass* ClassToCheck) const
{
	return 0;
}

bool UNAInventoryComponent::IsSlotFull(const FName& SlotID) const
{
	if (!IsEmptySlot(SlotID))
	{
		return CalculateNumberForFullSlotStack(SlotID) <= 0;
	}
	return false;
}

bool UNAInventoryComponent::IsOutOfSlot() const
{
	for (const auto& Pair : InventoryContents)
	{
		if (IsEmptySlot(Pair.Key))
		{
			return false;
		}
	}
	return true;
}

bool UNAInventoryComponent::IsInventoryFull() const
{
	for (const auto& Pair : InventoryContents)
	{
		if (IsEmptySlot(Pair.Key) || !IsSlotFull(Pair.Key))
		{
			return false;
		}
	}
	return true;
}

FName UNAInventoryComponent::FindMatchingItem(UNAItemData* InItem) const
{
	if (InItem)
	{
		for (const auto& Pair : InventoryContents)
		{
			if (!Pair.Value.IsValid())
			{
				continue;
			}

			if (Pair.Value.Get() == InItem)
			{
				return Pair.Key;
			}
		}
	}
	return NAME_None;
}

// UNAItemData* UNAInventoryComponent::FindNextPartialStack(UNAItemData* InItem) const
// {
// 	if (InItem)
// 	{
// 		if (const TArray<UNAItemData*>::ElementType* Result =
// 			InventoryContents.FindByPredicate([&InItem](const UNAItemData* InventoryItem)
// 			{
// 				return InventoryItem->GetItemID() == InItem->GetItemID() && !InventoryItem->IsFullItemStack();
// 			}))
// 		{
// 			return *Result;
// 		}
// 	}
// 	return nullptr;
// }

bool UNAInventoryComponent::HandleAddNewItem(UNAItemData* NewItemToAdd, const FName& SlotID)
{
	if (SlotID.IsNone() || !InventoryContents.Contains(SlotID))
	{
		ensure(false);
		return false;
	}
	UNAInventoryGameInstanceSubsystem* InvenSubsys = UNAInventoryGameInstanceSubsystem::Get(GetWorld());
	if (!InvenSubsys)
	{
		ensure(false);
		return false;
	}
	
	InventoryContents[SlotID] = NewItemToAdd;
	NewItemToAdd->SetOwningInventory(this);
	InvenSubsys->AddItemToInventory(this, SlotID, NewItemToAdd);
	return true;
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

