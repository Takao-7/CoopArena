// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Structs/ItemStructs.h"
#include "InventoryComponent.generated.h"


class AItemBase;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPARENA_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool AddItem(AItemBase* itemToAdd);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	int32 GetItemCountByClass(TSubclassOf<AItemBase> itemClass);
	
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool RemoveItemByIndex(int32 itemIndexToRemove, FItemStats& outItemStats);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool RemoveItemByClass(TSubclassOf<AItemBase> itemClass);
	
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool HasSpaceForItem(FItemStats& item) const;

	UFUNCTION(BlueprintPure, Category = Inventory)
	int32 GetInventorySize() const;

	/**
	 * Drops (= spawns) an item one meter in front of the owner.
	 * @param itemToDrop The item that should be dropped and spawned.
	 * @return The dropped item. Nullptr if the item couldn't be spawned for some reason.
	 */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	AItemBase* DropItem(FItemStats& itemToDrop);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

protected:
	/* The number of items that the inventory can hold */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Inventory)
	int32 _InventorySize;

	/* All items that are currently in this inventory */
	UPROPERTY(BlueprintReadWrite, Category = Inventory)
	TArray<FItemStats> _StoredItems;

	/* Items this inventory starts with. */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TArray<TSubclassOf<AItemBase>> _InitialItems;
};