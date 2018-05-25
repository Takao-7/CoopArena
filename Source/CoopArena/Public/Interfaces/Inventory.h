// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Interface.h"
#include "ItemBase.h"
#include "Inventory.generated.h"


UINTERFACE(BlueprintType)
class COOPARENA_API UInventory : public UInterface
{
	GENERATED_BODY()
	
};


class COOPARENA_API IInventory
{
	GENERATED_BODY()

public:
	/**
	* Adds an item to the inventory.
	* @param itemToAdd The item to be added to the inventory.
	* @return True if the item was successfully added.
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Inventory)
	bool AddItem(class AItemBase* itemToAdd);

	/**
	* Removes an item from the inventory. Does NOT spawn the item in the world!
	* @param itemIndexToRemove The item's index that should be removed from the inventory.
	* @return True if the item was successfully removed.
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Inventory)
	bool RemoveItem(int32 itemIndexToRemove, FItemStats& outItemStats);

	/**
	 * Returns true if the given item can be added to the inventory.
	 * If false, the item can not be added, ether because the inventory is full or does not accept the given item.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Inventory)
	bool HasSpaceForItem(FItemStats& item) const;

	/* Returns the number of items this inventory can store in total. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Inventory)
	int32 GetInventorySize() const;

	/* Returns true if a weapon can reload from this inventory (= directly take ammunition from it). */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Inventory)
	bool WeaponCanReloadFrom() const;
};