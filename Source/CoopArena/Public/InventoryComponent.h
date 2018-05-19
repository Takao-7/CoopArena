// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemBase.h"
#include "InventoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPARENA_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Adds an item to the inventory.
	 * @param itemToAdd The item to be added to the inventory.
	 * @return True if the item was successfully added.
	 */
	//UFUNCTION(BlueprintCallable, Category = Inventory)
	bool AddItem(class AItemBase* itemToAdd);

	/**
	* Removes an item from the inventory. Does NOT spawn the item in the world!
	* @param itemIndexToRemove The item's index that should be removed from the inventory.
	* @return True if the item was successfully removed.
	*/
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool RemoveItem(int32 itemIndexToRemove, FItemStats& outItemStats);

	/**
	 * Drops (= spawns) an item. If set, will use the defined Spawnpoint.
	 * If not existing, will spawn the item 1 meter in front of the owner.
	 * @param itemToDrop The item that should be dropped and spawned.
	 * @return The dropped item. Nullptr if the item couldn't be spawned for some reason.
	 */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	AItemBase* DropItem(FItemStats& itemToDrop);

	UFUNCTION(BlueprintPure, Category = Inventory)
	int32 GetInventorySize() const;

	UFUNCTION(BlueprintPure, Category = Inventory)
	bool HasSpaceForItem(FItemStats& item) const;

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

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Inventory)
	USceneComponent* _SpawnPoint;
};