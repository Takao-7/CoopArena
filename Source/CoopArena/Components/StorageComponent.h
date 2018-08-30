// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Structs/ItemStructs.h"
#include "StorageComponent.generated.h"


USTRUCT()
struct FStorageSlot
{
	GENERATED_BODY()

	FItemStats item;

	/* In m^3. Or, in case of non-splittable items, in whole items. */
	float amount;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPARENA_API UStorageComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UStorageComponent();	

	/* 
	 * Adds the given item to the storage.
	 * @return True if the item was successfully added.
	 * */
	UFUNCTION(BlueprintCallable, Category = Storage)
	bool AddItem(FItemStats& ItemToAdd, float Amount);

	/*
	 * @param ItemName The item name to look for.
	 * @return How much (or many) of the given item is in this storage.
	 */
	UFUNCTION(BlueprintCallable, Category = Storage)
	float GetItemStackSize(FName ItemName);

	/* 
	 * Removes an amount of the given item from the storage.
	 * @param ItemName The item to remove.
	 * @param Amount how much to remove. A value of <= 0 will remove the entire item stack.
	 * @param OutRemovedItem The removed item.
	 * @return The amount that was removed.
	 */
	UFUNCTION(BlueprintCallable, Category = Storage)
	float RemoveItemByName(FName ItemName, float Amount, FItemStats& OutRemovedItem);

	/*
	 * Removes an item from the storage.
	 * @param ItemToRemove The item and amount to remove.
	 * @param OutRemovedItem The removed item.
	 * @return The amount that was removed.
	 */
	UFUNCTION(BlueprintCallable, Category = Storage)
	float RemoveItem(FItemStats& ItemToRemove, FItemStats& OutRemovedItem);

	/* Returns the entire storage. */
	UFUNCTION(BlueprintPure, Category = Storage)
	FORCEINLINE TArray<FItemStats> GetAllItems();

protected:
	virtual void BeginPlay() override;

	FORCEINLINE FItemStats* FindItem(FName ItemName);

	/**
	 * How much, in kg, this storage can hold.
	 * Set to 0 for no weight limit.
	 */
	UPROPERTY(EditDefaultsOnly, Category = Storage)
	float _Capacity;

	/**
	 * How much, in m^3, volume this storage can hold.
	 * Set to 0 for no volume limit.
	 */
	UPROPERTY(EditDefaultsOnly, Category = Storage)
	float _Volume;

	/* How much, in kg, does this storage contains. */
	UPROPERTY(BlueprintReadOnly, Category = Storage)
	float _CurrentLoad;

	/* How much volume, in m^3, does this storage contains. */
	UPROPERTY(BlueprintReadOnly, Category = Storage)
	float _CurrentVolume;

	/* The type and amount of item that this container should spawn with. */
	UPROPERTY(EditDefaultsOnly, Category = Storage)
	TMap<TSubclassOf<AItemBase>, float> _ItemsToSpawnWith;

	UPROPERTY(BlueprintReadOnly, Category = Storage)
	TArray<FStorageSlot> _StoredItems;
};