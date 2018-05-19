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
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool AddItem(class AItemBase* itemToAdd);

	UFUNCTION(BlueprintPure, Category = Inventory)
	int32 GetInventorySize() const;

	UFUNCTION(BlueprintPure, Category = Inventory)
	bool HasSpaceForItem(FItemStats item) const;

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
};