// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"
#include "Gameframework/Pawn.h"
#include "Engine/World.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "ItemBase.h"


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	AddDefaultItems();
}


/////////////////////////////////////////////////////
void UInventoryComponent::AddDefaultItems()
{
	_StoredItems.Empty();
	for (TSubclassOf<AItemBase> itemClass : _InitialItems)
	{
		FItemStats itemStats = Cast<AItemBase>(itemClass->GetDefaultObject())->GetItemStats();
		_StoredItems.Add(itemStats);
		IncreaseMagazineCount(itemStats);
	}
	_InitialItems.Empty();
}


/////////////////////////////////////////////////////
bool UInventoryComponent::AddItem(AItemBase* itemToAdd)
{
	FItemStats itemstats = itemToAdd->GetItemStats();
	if (!HasSpaceForItem(itemstats))
	{
		return false;
	}
	_StoredItems.Add(itemstats);
	IncreaseMagazineCount(itemstats);

	return true;
}


/////////////////////////////////////////////////////
void UInventoryComponent::IncreaseMagazineCount(FItemStats &itemstats)
{
	if (itemstats.type == EItemType::Ammo)
	{
		auto numMags = _StoredMagazines.Find(itemstats.name);
		if (numMags == nullptr)
		{
			_StoredMagazines.Add(itemstats.name, 0);
		}
		_StoredMagazines[itemstats.name]++;
	}
}


void UInventoryComponent::ReduceMagazineCount(FItemStats &outItemStats)
{
	if (outItemStats.type == EItemType::Ammo)
	{
		_StoredMagazines[outItemStats.name]--;
	}
}


/////////////////////////////////////////////////////
bool UInventoryComponent::RemoveItemByIndex(int32 itemIndexToRemove, FItemStats& outItemStats)
{
	bool bItemExists = _StoredItems.IsValidIndex(itemIndexToRemove);
	if (!bItemExists)
	{
		return false;
	}
	outItemStats = _StoredItems[itemIndexToRemove];
	_StoredItems.RemoveAt(itemIndexToRemove);
	ReduceMagazineCount(outItemStats);

	return true;
}


bool UInventoryComponent::RemoveItemByClass(TSubclassOf<AItemBase> itemClass)
{
	AItemBase* itemToCompare = Cast<AItemBase>(itemClass->GetDefaultObject());
	FItemStats statsToCompare = itemToCompare->GetItemStats();
	for (int i = 0; i < _StoredItems.Num(); i++)
	{
		if (_StoredItems[i].name == statsToCompare.name)
		{
			_StoredItems.RemoveAt(i);
			ReduceMagazineCount(statsToCompare);
			return true;
		}
	}
	return false;
}


/////////////////////////////////////////////////////
AItemBase* UInventoryComponent::DropItem(FItemStats& itemToDrop)
{
	FVector spawnLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 100.0f;
	FActorSpawnParameters spawnParameters;
	spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	AItemBase* spawnedItem = GetWorld()->SpawnActor<AItemBase>(itemToDrop.itemClass, spawnLocation, FRotator::ZeroRotator, spawnParameters);
	if (spawnedItem)
	{
		spawnedItem->SetItemStats(itemToDrop);
	}

	return spawnedItem;
}


/////////////////////////////////////////////////////
int32 UInventoryComponent::GetInventorySize() const
{
	return _InventorySize;
}


bool UInventoryComponent::HasSpaceForItem(FItemStats& item) const
{
	bool bInventoryNotFull = _StoredItems.Num() < _InventorySize;

	return bInventoryNotFull;
}


int32 UInventoryComponent::GetItemCountByClass(TSubclassOf<AItemBase> itemClass) const
{
	int32 count = 0;
	AItemBase* itemToCompare = Cast<AItemBase>(itemClass->GetDefaultObject());
	FItemStats statsToCompare = itemToCompare->GetItemStats();
	for (FItemStats item : _StoredItems)
	{
		if (item.name == statsToCompare.name)
		{
			count++;
		}
	}
	return count;
}


int32 UInventoryComponent::GetMagazineCountByName(FName magazineName) const
{
	return _StoredMagazines[magazineName];
}
