// Fill out your copyright notice in the Description page of Project Settings.

#include "StorageComponent.h"
#include "ItemBase.h"
<<<<<<< HEAD
=======
#include "UnrealNetwork.h"
#include "Engine/NetDriver.h"
#include "Net/RepLayout.h"
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8


UStorageComponent::UStorageComponent()
{
	_WeightLimit = -1;
	_Capacity = -1;
}


/////////////////////////////////////////////////////
			/* Alter items in Storage */
/////////////////////////////////////////////////////
int32 UStorageComponent::ChangeItemStack(FItemStats& Item, int32 AmountToChange)
{
	if (AmountToChange == 0)
	{
		return 0;
	}

	int32 changedAmount = 0;
	if (AmountToChange > 0)
	{
		bool bItemAdded = AddItem(Item, AmountToChange);
		if (bItemAdded)
		{
			return AmountToChange;
		}
		else
		{
			return 0; // Inventory is full, nothing was added.
		}
	}
	else
	{
		changedAmount = RemoveItem(Item, -AmountToChange);
		return changedAmount;
	}	
}

<<<<<<< HEAD

=======
/////////////////////////////////////////////////////
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
bool UStorageComponent::AddItem(FItemStats& ItemToAdd, int32 Amount)
{
	bool bCorrectAmountValue = Amount > 0;
	bool bEnoughFreeWeight = CheckWeight(ItemToAdd.weight);
	bool bEnoughCapacity = CheckCapacity(Amount);
	if(!bCorrectAmountValue || !bEnoughFreeWeight || !bEnoughCapacity)
	{
		return false;
	}
		
	FItemStack* itemInStorage = FindItem(ItemToAdd.name);
	itemInStorage ? itemInStorage->ChangeStackSize(Amount) : _StoredItems.Add(FItemStack(ItemToAdd, Amount));

	_CurrentVolume += ItemToAdd.volume * Amount;
	_CurrentWeight += ItemToAdd.weight * Amount;

	return true;
}


int32 UStorageComponent::RemoveItem(FItemStats& ItemToRemove, int32 Amount)
{
	FItemStack* itemStack = FindItem(ItemToRemove.name);
	if (itemStack == nullptr || Amount == 0 || (Amount < 0 && Amount != -1))
	{
		return 0;
	}

	int32 changedAmount = Amount;
	if (Amount == -1)
	{
		changedAmount = itemStack->GetStackSize();
		_StoredItems.RemoveSingleSwap(*itemStack);
	}
	else
	{
		changedAmount = itemStack->ChangeStackSize(-changedAmount);
		if (itemStack->GetStackSize() == 0)
		{
			_StoredItems.RemoveSingleSwap(*itemStack);
		}
	}

	_CurrentVolume -= ItemToRemove.volume * changedAmount;
	_CurrentWeight -= ItemToRemove.weight * changedAmount;

	return changedAmount;
}


/////////////////////////////////////////////////////
					/* Checks */
/////////////////////////////////////////////////////
bool UStorageComponent::CheckCapacity(float VolumeToAdd)
{
	if (_Capacity != -1)
	{
		if (_CurrentVolume + VolumeToAdd > _Capacity)
		{
			// #TODO: Add functionality for adding just the amount that fits into the storage.
			return false;
		}
	}

	return true;
}

<<<<<<< HEAD

=======
/////////////////////////////////////////////////////
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
bool UStorageComponent::CheckWeight(float WeightToAdd)
{
	if (_WeightLimit != -1)
	{
		if (_CurrentWeight + WeightToAdd > _WeightLimit)
		{
			// #TODO: Add functionality for adding just the amount that fits into the storage.
			return false;
		}		
	}	

	return true;
}

<<<<<<< HEAD

=======
/////////////////////////////////////////////////////
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
bool UStorageComponent::HasItem(FItemStats& Item)
{
	bool bHasItem = false;
	FindItem(Item.name) ? bHasItem = true : bHasItem = false;
	return bHasItem;
}

<<<<<<< HEAD

=======
/////////////////////////////////////////////////////
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
void UStorageComponent::CheckWeightLimitAndCapacity() const
{
	bool bCorrectSetWeightLimit = _WeightLimit == -1 || _WeightLimit >= 0;
	bool bCorrectSetCapacity = _Capacity == -1 || _Capacity >= 0;
	if (!bCorrectSetCapacity || !bCorrectSetWeightLimit)
	{
		UE_LOG(LogTemp, Fatal, TEXT("WeightLimit or Capacity on %s isn't correctly set! WeightLimit: %f. Capacity: %f"), *GetName(), _WeightLimit, _Capacity);
	}
}


/////////////////////////////////////////////////////
					/* Getter */
/////////////////////////////////////////////////////
int32 UStorageComponent::GetItemStackSize(FName ItemName)
{
	FItemStack* itemInStorage = FindItem(ItemName);
	if (itemInStorage)
	{
		return itemInStorage->GetStackSize();
	}
	else
	{
		return 0;
	}
}

<<<<<<< HEAD

=======
/////////////////////////////////////////////////////
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
TArray<FItemStack> UStorageComponent::GetStorageCopy() const
{
	return _StoredItems;
}

<<<<<<< HEAD

=======
/////////////////////////////////////////////////////
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
FItemStack* UStorageComponent::FindItem(FName ItemName)
{
	FItemStack* foundStack = _StoredItems.FindByPredicate([ItemName](FItemStack stack) {return stack.item.name == ItemName; });
	return foundStack;
}


/////////////////////////////////////////////////////
				/* Misc functions */
/////////////////////////////////////////////////////
void UStorageComponent::PrintInventory()
{
	for (FItemStack& stack : _StoredItems)
	{
		FString itemName = stack.item.name.ToString();
		UE_LOG(LogTemp, Warning, TEXT("Item: %n. Amount: %f"), *itemName, stack.GetStackSize());
	}
}

<<<<<<< HEAD

void UStorageComponent::BeginPlay()
{
	Super::BeginPlay();
	CheckWeightLimitAndCapacity();
	AddStartingItems();
}


=======
/////////////////////////////////////////////////////
void UStorageComponent::BeginPlay()
{
	Super::BeginPlay();

	if(GetOwner()->HasAuthority())
	{
		CheckWeightLimitAndCapacity();
		AddStartingItems();

		//if (GetOwner()->GetInstigator() == nullptr)
		//{
		//	// We are not attached to a pawn, so we are a container and will replicate our stored items to everyone.
		//	UNetDriver* netDriver = GetOwner()->GetNetDriver();
		//	if (netDriver)
		//	{
		//		netDriver->FindOrCreateRepChangedPropertyTracker(this).Get()->SetCustomIsActiveOverride(COND_Max, true);
		//	}
		//}
	}
}

/////////////////////////////////////////////////////
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
void UStorageComponent::AddStartingItems()
{
	if (_ItemsToSpawnWith.Num() > 0)
	{
		for (auto& item : _ItemsToSpawnWith)
		{
			AItemBase* itemBase = Cast<AItemBase>(item.Key->GetDefaultObject(true));

			uint32 amount = item.Value;
			FItemStats stats = itemBase->GetItemStats();

			AddItem(stats, amount);
		}
	}
<<<<<<< HEAD
=======
}


/////////////////////////////////////////////////////
				/* Networking */
/////////////////////////////////////////////////////
void UStorageComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UStorageComponent, _StoredItems, COND_Custom);
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
}