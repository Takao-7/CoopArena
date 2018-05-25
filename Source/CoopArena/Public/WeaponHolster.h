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
	/* IInventory interface */
	virtual bool AddItem_Implementation(AItemBase* itemToAdd) override;
	virtual bool RemoveItem_Implementation(int32 itemIndexToRemove, FItemStats& outItemStats) override;
	virtual bool HasSpaceForItem_Implementation(FItemStats& item) const override;
	virtual int32 GetInventorySize_Implementation() const override;
	virtual bool WeaponCanReloadFrom_Implementation() const override;
	/* IInventory interface end */

protected:
	UFUNCTION(BlueprintCallable, Category = WeaponHolster)
	void AttachGunToHolster(AGun* GunToAttach);

protected:
	AGun* _CarriedGun;	

	UPROPERTY(EditDefaultsOnly, Category = WeaponHolster)
	UAnimMontage* _HolsteringAnimations;	
};
