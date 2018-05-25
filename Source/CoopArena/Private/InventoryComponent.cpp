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
}


bool UInventoryComponent::AddItem_Implementation(AItemBase* itemToAdd)
{
	if (itemToAdd == nullptr)
	{
		return false;
	}

	FItemStats itemstats = itemToAdd->GetItemStats();
	if (!HasSpaceForItem_Implementation(itemstats))
	{
		return false;
	}
	_StoredItems.Add(itemstats);
	return true;
}


bool UInventoryComponent::RemoveItem_Implementation(int32 itemIndexToRemove, FItemStats& outItemStats)
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


int32 UInventoryComponent::GetInventorySize_Implementation() const
{
	return _InventorySize;
}


bool UInventoryComponent::WeaponCanReloadFrom_Implementation() const
{
	return bWeaponsCanReloadFrom;
}

bool UInventoryComponent::HasSpaceForItem_Implementation(FItemStats& item) const
{
	bool bInventoryNotFull = _StoredItems.Num() < _InventorySize;

	return bInventoryNotFull;
}
