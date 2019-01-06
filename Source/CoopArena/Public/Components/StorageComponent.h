// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemBase.h"
#include "StorageComponent.generated.h"


USTRUCT(BlueprintType)
struct FItemStack
{
	GENERATED_BODY()

public:
	FItemStack() {};

	/* Creates a new stack based on a given item and a stack size. */
	FItemStack(FItemStats& Item, uint32 StackSize)
	{
		this->item = Item;
		this->stackSize = StackSize;
	};

	UPROPERTY(BlueprintReadWrite)
	FItemStats item;

	FORCEINLINE bool operator==(const FItemStack& otherStack) const
	{
		return otherStack.item.name == item.name;
	}

	/**
	 * Changes the stack size.
	 * @param AmountToChange The amount to change the stack size by.
	 * A positive value will increase the stack, a negative will decrease it.
	 * @return The amount that the stack was actually changed by (the stack size can't be smaller than 0).
	 */
	int32 ChangeStackSize(int32 AmountToChange)
	{
		int32 changedAmount = AmountToChange;
		if (FMath::Abs(changedAmount) > stackSize)
		{
			changedAmount = -stackSize;
		}
		stackSize += changedAmount;
		return FMath::Abs(changedAmount);
	}

	int32 GetStackSize() const	{ return stackSize;	}

private:
	/* How many items are in this stack. */
	UPROPERTY()
	int32 stackSize;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class COOPARENA_API UStorageComponent : public UActorComponent
{
	GENERATED_BODY()


	/////////////////////////////////////////////////////
				/* Parameters & variables */
	/////////////////////////////////////////////////////
protected:
	/**
	 * How much weight, in kg, this storage can hold.
	 * Set to -1 for no weight limit.
	 */
	UPROPERTY(EditDefaultsOnly, Category = Storage)
	float _WeightLimit;

	/**
	 * How much volume, in cm^3, this storage can hold.
	 * Set to -1 for no volume limit.
	 */
	UPROPERTY(EditDefaultsOnly, Category = Storage)
	float _Capacity;

	/* The type and amount of item that this container should spawn with. */
	UPROPERTY(EditDefaultsOnly, Category = Storage)
	TMap<TSubclassOf<AItemBase>, int32> _ItemsToSpawnWith;

	/* How much, in kg, does this storage contains. */
	UPROPERTY(BlueprintReadOnly, Category = Storage)
	float _CurrentWeight;

	/* How much volume, in cm^3, is occupied in this storage. */
	UPROPERTY(BlueprintReadOnly, Category = Storage)
	float _CurrentVolume;

	/* All items that are currently in this storage. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = Storage)
	TArray<FItemStack> _StoredItems;


	/////////////////////////////////////////////////////
				/* Alter items in Storage */
	/////////////////////////////////////////////////////
public:
	/**
	 * Changes the item stack by adding or removing the given amount.
	 * @param Item The item.
	 * @param AmountToChange The volume, m^3 (or whole pieces in case of non-splittable items) that the given item should be changed by.
	 * A positive value means adding that amount to the storage, a negative will remove that amount.
	 * If the given item is not in this storage, it will be added to it, if Amount is positive.
	 * @return The actual amount that was changed. 0 means that nothing has changed.
	 */
	UFUNCTION(BlueprintCallable, Category = Storage)
	int32 ChangeItemStack(FItemStats& Item, int32 AmountToChange);

	/** 
	 * Adds the given item to the storage, or, when the item is already in storage, adds the given amount to it.
	 * @param ItemToAdd The item to add to this storage.
	 * @param Amount How much to add to the inventory. Must be > 0.
	 * @return True if the amount was successfully added.
	 */
	UFUNCTION(BlueprintCallable, Category = Storage)
	bool AddItem(FItemStats& ItemToAdd, int32 Amount);

	/**
	 * Removes an item from the storage.
	 * @param ItemToRemove The item and amount to remove.
	 * @param Amount How much (or many) of the given item should be removed.
	 * Set to -1 to remove the entire stack in this storage.
	 * @return The amount that was actually removed. 0 means that the given item is not in this storage or Amount was 0.
	 */
	UFUNCTION(BlueprintCallable, Category = Storage)
	int32 RemoveItem(FItemStats& ItemToRemove, int32 Amount);


	/////////////////////////////////////////////////////
						/* Checks */
	/////////////////////////////////////////////////////
public:
	/* Checks if this storage has enough free volume for the given volume. */
	UFUNCTION(BlueprintCallable, Category = Storage)
	bool CheckCapacity(float VolumeToAdd);

	/* Checks if this storage can hold the given weight. */
	UFUNCTION(BlueprintCallable, Category = Storage)
	bool CheckWeight(float WeightToAdd);

	/* Checks if the inventory contains the given item at any quantity and returns true if that is the case. */
	UFUNCTION(BlueprintCallable, Category = Storage)
	bool HasItem(FItemStats& Item);

protected:
	/* Checks if the weight limit and capacity is set correctly. Will crash if not. */
	UFUNCTION(BlueprintCallable, Category = Storage)
	void CheckWeightLimitAndCapacity() const;


	/////////////////////////////////////////////////////
						/* Getter */
	/////////////////////////////////////////////////////
public:
	/**
	 * @param ItemName The item name to look for.
	 * @return How much (or many) of the given item is in this storage.
	 */
	UFUNCTION(BlueprintCallable, Category = Storage)
	int32 GetItemStackSize(FName ItemName);	

	/* Returns a copy of the entire storage. */
	UFUNCTION(BlueprintPure, Category = Storage)
	TArray<FItemStack> GetStorageCopy() const;

protected:
	FItemStack* FindItem(FName ItemName);


	/////////////////////////////////////////////////////
					/* Misc functions */
	/////////////////////////////////////////////////////
public:
	UStorageComponent();

	UFUNCTION(BlueprintCallable, Category = Storage)
	void PrintInventory();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = Storage)
	void AddStartingItems();	
};