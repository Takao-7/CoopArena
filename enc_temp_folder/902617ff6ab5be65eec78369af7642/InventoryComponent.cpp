// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"
#include "Gameframework/Pawn.h"
#include "Engine/World.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "ItemBase.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	_StoredItems.Empty();
	for (TSubclassOf<AItemBase> itemClass : _InitialItems)
	{
		FItemStats itemStats = Cast<AItemBase>(itemClass->GetDefaultObject())->GetItemStats();
		//UE_LOG(LogTemp, Warning, TEXT("%s"), *itemStats.Name.ToString());
		_StoredItems.Add(itemStats);
	}
}


bool UInventoryComponent::AddItem(AItemBase* itemToAdd)
{
	FItemStats itemstats = itemToAdd->GetItemStats();
	if (!HasSpaceForItem(itemstats))
	{
		return false;
	}
	_StoredItems.Add(itemstats);
	return true;
}


int32 UInventoryComponent::GetItemCountByClass(TSubclassOf<AItemBase> itemClass)
{
	int32 count = 0;
	AItemBase* itemToCompare = Cast<AItemBase>(itemClass->GetDefaultObject());
	FItemStats statsToCompare = itemToCompare->GetItemStats();
	for (FItemStats item : _StoredItems)
	{
		if (item.Name == statsToCompare.Name)
		{
			count++;
		}
	}
	return count;
}


bool UInventoryComponent::RemoveItemByIndex(int32 itemIndexToRemove, FItemStats& outItemStats)
{
	bool bItemExists = _StoredItems.IsValidIndex(itemIndexToRemove);
	if (!bItemExists)
	{
		return false;
	}
	
	outItemStats = _StoredItems[itemIndexToRemove];
	_StoredItems.RemoveAt(itemIndexToRemove);
	return true;
}


bool UInventoryComponent::RemoveItemByClass(TSubclassOf<AItemBase> itemClass)
{
	AItemBase* itemToCompare = Cast<AItemBase>(itemClass->GetDefaultObject());
	FItemStats statsToCompare = itemToCompare->GetItemStats();
	for (int i = 0; i < _StoredItems.Num(); i++)
	{
		if (_StoredItems[i].Name == statsToCompare.Name)
		{
			_StoredItems.RemoveAt(i);
			//UE_LOG(LogTemp, Warning, TEXT("New inventory size: %s"), _StoredItems.Num());
			return true;
		}
	}
	return false;
}


AItemBase* UInventoryComponent::DropItem(FItemStats& itemToDrop)
{
	FVector spawnLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 100.0f;
	FActorSpawnParameters spawnParameters;
	spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	AItemBase* spawnedItem = GetWorld()->SpawnActor<AItemBase>(itemToDrop.Class, spawnLocation, FRotator::ZeroRotator, spawnParameters);
	if (spawnedItem)
	{
		spawnedItem->SetItemStats(itemToDrop);
	}

	return spawnedItem;
}


int32 UInventoryComponent::GetInventorySize() const
{
	return _InventorySize;
}


bool UInventoryComponent::HasSpaceForItem(FItemStats& item) const
{
	bool bInventoryNotFull = _StoredItems.Num() < _InventorySize;

	return bInventoryNotFull;
}
