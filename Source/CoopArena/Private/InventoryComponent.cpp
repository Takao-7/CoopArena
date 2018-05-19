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
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}


bool UInventoryComponent::AddItem(class AItemBase* itemToAdd)
{
	if (!itemToAdd)
	{
		return false;
	}

	FItemStats itemstats = itemToAdd->GetItemStats();
	if (!HasSpaceForItem(itemstats))
	{
		return false;
	}
	_StoredItems.Add(itemstats);
	return true;
}


bool UInventoryComponent::RemoveItem(int32 itemIndexToRemove, FItemStats& outItemStats)
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
	FVector spawnLocation;
	if (_SpawnPoint)
	{
		spawnLocation = _SpawnPoint->GetComponentLocation();
	}
	else
	{
		spawnLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 100.0f;
	}
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
