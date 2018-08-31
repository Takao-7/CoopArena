// Fill out your copyright notice in the Description page of Project Settings.

#include "StorageComponent.h"
#include "ItemBase.h"


UStorageComponent::UStorageComponent()
{
	_WeightLimit = -1.0f;
	_Capacity = -1.0f;
}


/////////////////////////////////////////////////////
			/* Alter items in Storage */
/////////////////////////////////////////////////////
float UStorageComponent::ChangeItemStack(FItemStats& Item, float AmountToChange)
{
	if (AmountToChange == 0.0f)
	{
		return 0.0f;
	}

	float changedAmount = 0.0f;
	if (AmountToChange > 0.0f)
	{
		bool bItemAdded = AddItem(Item, AmountToChange);
		if (bItemAdded)
		{
			changedAmount = AmountToChange;
			return changedAmount;
		}
		else
		{
			return 0.0f;
		}
	}
	else
	{
		changedAmount = RemoveItem(Item, -AmountToChange);
		return changedAmount;
	}	
}


bool UStorageComponent::AddItem(FItemStats& ItemToAdd, float Amount)
{
	bool bCorrectAmountValue = Amount > 0.0f;
	bool bEnoughFreeWeight = CheckWeight(ItemToAdd.density, Amount);
	bool bEnoughCapacity = CheckCapacity(Amount);
	if(!bCorrectAmountValue || !bEnoughFreeWeight || !bEnoughCapacity)
	{
		return false;
	}
		
	FItemStack* itemInStorage = FindItem(ItemToAdd.name);
	itemInStorage ? itemInStorage->ChangeAmount(Amount) : _StoredItems.Add(FItemStack(ItemToAdd, Amount));

	ItemToAdd.bIsNotSplittable ? _CurrentVolume += ItemToAdd.volume * Amount : _CurrentVolume += Amount;
	_CurrentWeight += ItemToAdd.density * Amount;

	return true;
}


float UStorageComponent::RemoveItem(FItemStats& ItemToRemove, float Amount)
{
	FItemStack* itemStack = FindItem(ItemToRemove.name);
	if (itemStack == nullptr || Amount == 0.0f || (Amount < 0.0f && Amount != -1.0f))
	{
		return 0.0f;
	}

	float changedAmount = 0.0f;
	if (Amount == -1.0f)
	{
		changedAmount = itemStack->GetStackSize();
		_StoredItems.RemoveSingleSwap(*itemStack);
	}
	else
	{
		changedAmount = itemStack->ChangeAmount(-Amount);
		if (itemStack->GetStackSize() == 0.0f)
		{
			_StoredItems.RemoveSingleSwap(*itemStack);
		}
	}

	ItemToRemove.bIsNotSplittable ? _CurrentVolume -= ItemToRemove.volume * changedAmount : _CurrentVolume -= changedAmount;
	_CurrentWeight -= ItemToRemove.density * changedAmount;

	return changedAmount;
}


/////////////////////////////////////////////////////
					/* Checks */
/////////////////////////////////////////////////////
bool UStorageComponent::CheckCapacity(float Amount)
{
	if (_Capacity != -1.0f)
	{
		if (_CurrentVolume + Amount > _Capacity)
		{
			// #TODO: Add functionality for adding just the amount that fits into the storage.
			return false;
		}
	}

	return true;
}


bool UStorageComponent::CheckWeight(float Density, float Amount)
{
	if (_WeightLimit != -1.0f)
	{
		float addedWeight = Density * Amount;
		if (_CurrentWeight + addedWeight > _WeightLimit)
		{
			// #TODO: Add functionality for adding just the amount that fits into the storage.
			return false;
		}		
	}	

	return true;
}


bool UStorageComponent::HasItem(FItemStats& Item)
{
	bool bHasItem = false;
	FindItem(Item.name) ? bHasItem = true : bHasItem = false;
	return bHasItem;
}


void UStorageComponent::CheckWeightLimitAndCapacity() const
{
	bool bCorrectSetWeightLimit = _WeightLimit == -1.0f || _WeightLimit >= 0.0f;
	bool bCorrectSetCapacity = _Capacity == -1.0f || _Capacity >= 0.0f;
	if (!bCorrectSetCapacity || !bCorrectSetWeightLimit)
	{
		UE_LOG(LogTemp, Fatal, TEXT("WeightLimit or Capacity on %s isn't correctly set! WeightLimit: %f. Capacity: %f"), *GetName(), _WeightLimit, _Capacity);
	}
}


/////////////////////////////////////////////////////
					/* Getter */
/////////////////////////////////////////////////////
float UStorageComponent::GetItemStackSize(FName ItemName)
{
	FItemStack* itemInStorage = FindItem(ItemName);
	if (itemInStorage)
	{
		return itemInStorage->GetStackSize();
	}
	else
	{
		return 0.0f;
	}
}


TArray<FItemStack> UStorageComponent::GetStorageCopy() const
{
	return _StoredItems;
}


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


void UStorageComponent::BeginPlay()
{
	Super::BeginPlay();
	CheckWeightLimitAndCapacity();
	AddStartingItems();
}


void UStorageComponent::AddStartingItems()
{
	if (_ItemsToSpawnWith.Num() > 0)
	{
		for (auto& item : _ItemsToSpawnWith)
		{
			AItemBase* itemBase = Cast<AItemBase>(item.Key->GetDefaultObject(true));

			float amount = item.Value;
			FItemStats stats = itemBase->GetItemStats();

			AddItem(stats, amount);
		}
	}
}