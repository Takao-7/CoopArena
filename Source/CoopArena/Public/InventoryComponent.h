// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemBase.h"
#include "Inventory.h"
#include "InventoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPARENA_API UInventoryComponent : public UActorComponent, public IInventory
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	/* IInventory interface */
	virtual bool AddItem_Implementation(class AItemBase* itemToAdd);
	virtual bool RemoveItem_Implementation(int32 itemIndexToRemove, FItemStats& outItemStats);
	virtual bool HasSpaceForItem_Implementation(FItemStats& item) const;
	virtual int32 GetInventorySize_Implementation() const;
	virtual bool WeaponCanReloadFrom_Implementation() const;
	/* IInventory interface end */

	/**
	 * Drops (= spawns) an item. If set, will use the defined Spawnpoint.
	 * If not existing, will spawn the item 1 meter in front of the owner.
	 * @param itemToDrop The item that should be dropped and spawned.
	 * @return The dropped item. Nullptr if the item couldn't be spawned for some reason.
	 */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	virtual AItemBase* DropItem(FItemStats& itemToDrop);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

protected:
	/* The number of items that the inventory can hold */
	UPROPERTY(BlueprintReadWrite, Category = Inventory)
	int32 _InventorySize;

	/* All items that are currently in this inventory */
	UPROPERTY(BlueprintReadWrite, Category = Inventory)
	TArray<FItemStats> _StoredItems;

	/** 
	 * If true, weapons can reload from this inventory component.
	 * If a pawn doesn't have any inventory component with this set to true, the pawn is not able to reload his weapons.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Inventory)
	bool bWeaponsCanReloadFrom;
};