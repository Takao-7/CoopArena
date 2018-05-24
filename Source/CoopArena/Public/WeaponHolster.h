// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Inventory.h"
#include "WeaponHolster.generated.h"


class AGun;
class UAnimMontage;
class AItemBase;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPARENA_API UWeaponHolster : public USceneComponent, public IInventory
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponHolster();

	/* IInventory interface */
	virtual bool AddItem_Implementation(AItemBase* itemToAdd);
	virtual bool RemoveItem_Implementation(int32 itemIndexToRemove, FItemStats& outItemStats);
	virtual bool HasSpaceForItem_Implementation(FItemStats& item) const;
	virtual int32 GetInventorySize_Implementation() const;
	virtual bool WeaponCanReloadFrom_Implementation() const;
	/* IInventory interface end */

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = WeaponHolster)
	void AttachGunToHolster(AGun* GunToAttach);
protected:
	AGun* _CarriedGun;	

	UAnimMontage* _HolsteringAnimations;
	
};
