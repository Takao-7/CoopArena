// Fill out your copyright notice in the Description page of Project Settings.

#include "StorageComponent.h"


UStorageComponent::UStorageComponent()
{
	_Capacity = 10.0f;
}


void UStorageComponent::BeginPlay()
{
	Super::BeginPlay();

	if (_ItemsToSpawnWith.Num() > 0)
	{
		
	}
}


bool UStorageComponent::AddItem(FItemStats& ItemToAdd, float Amount)
{
	if (_Capacity != 0.0f)
	{
		float addedWeight = ItemToAdd.density * Amount;
		if (_CurrentLoad + addedWeight > _Capacity)
		{
			// TODO: Add functionality for adding just the amount that fits into the storage.
			return false;
		}
	}
	if (_Volume != 0.0f)
	{
		if (_CurrentVolume + Amount > _Volume)
		{
			// TODO: Add functionality for adding just the amount that fits into the storage.
			return false;
		}
	}

	
	FItemStats* itemInStorage = FindItem(ItemToAdd.name);
	if (itemInStorage)
	{
		itemInStorage->ChangeAmount(itemInStorage->GetStackSize());		
	}
	else
	{
		_StoredItems.Add(ItemToAdd);
	}

	return true;
}


float UStorageComponent::GetItemStackSize(FName ItemName)
{
	FItemStats* itemInStorage = FindItem(ItemName);
	if (itemInStorage)
	{
		return itemInStorage->GetStackSize();
	}
	else
	{
		return 0.0f;
	}
}


FItemStats* UStorageComponent::FindItem(FName ItemName)
{
	return _StoredItems.FindByPredicate([ItemName](FItemStats item) {return item.name == ItemName; });
}


float UStorageComponent::RemoveItemByName(FName ItemName, float Amount, FItemStats& OutRemovedItem)
{
	FItemStats* itemInStorage = FindItem(ItemName);
	if (itemInStorage == nullptr)
	{
		return 0.0f;
	}

	float amountRemoved = Amount;
	if (Amount <= itemInStorage->GetStackSize())
	{
		OutRemovedItem = FItemStats(*itemInStorage, Amount);
	}
	else
	{
		OutRemovedItem = *itemInStorage;
		amountRemoved = itemInStorage->GetStackSize();
	}
	_StoredItems.RemoveAll([itemInStorage](FItemStats item) {return itemInStorage->name == item.name; });
	return amountRemoved;
}


float UStorageComponent::RemoveItem(FItemStats& ItemToRemove, FItemStats& OutRemovedItem)
{
	FItemStats* itemInStorage = FindItem(ItemToRemove.name);
	if (itemInStorage == nullptr)
	{
		return 0.0f;
	}

	if (ItemToRemove.GetStackSize() >= itemInStorage->GetStackSize())
	{
		OutRemovedItem = *itemInStorage;
	}
	else
	{
		FItemStats newItemStack = FItemStats(ItemToRemove, ItemToRemove.GetStackSize());
		OutRemovedItem = newItemStack;
	}
	
	_StoredItems.RemoveAll([itemInStorage](FItemStats item) {return itemInStorage->name == item.name; });
	return itemInStorage->GetStackSize();
}


TArray<FItemStats> UStorageComponent::GetAllItems()
{
	return _StoredItems;
}
