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

	UFUNCTION(BlueprintCallable, Category = WeaponHolster)
	void ToggleIsInUse();

	UFUNCTION(BlueprintPure, Category = WeaponHolster)
	bool IsInUse() { return bIsInUse; };

	UWeaponHolster();

	UFUNCTION(BlueprintPure, Category = WeaponHolster)
	AGun* GetCarriedGun() { return _CarriedGun; };

	/**
	 * Function to be called from the animation blueprint when ether a gun should be attached or detached from this holster. 
	 * AddItem or RemoveItem needed to be called first.
	 */ 
	UFUNCTION(BlueprintCallable, Category = WeaponHolster)
	void OnAttachAndDetach();

protected:
	void PlayEquipAnimation();

	UFUNCTION(BlueprintCallable, Category = WeaponHolster)
	void AttachGunToHolster();

	UFUNCTION(BlueprintCallable, Category = WeaponHolster)
	void DetachGunFromHolster();

protected:
	AGun* _CarriedGun;	

	UPROPERTY(EditDefaultsOnly, Category = WeaponHolster)
	UAnimMontage* _HolsteringAnimations;	

	/* Socket name where the carried gun will be attached to. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = WeaponHolster)
	FName _AttachPoint;

	UPROPERTY()
	bool bIsInUse;
};
