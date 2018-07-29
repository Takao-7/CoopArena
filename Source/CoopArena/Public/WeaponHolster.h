// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Inventory.h"
#include "Interactable.h"
#include "WeaponEnums.h"
#include "WeaponHolster.generated.h"


class AGun;
class UAnimMontage;
class AItemBase;
class AHumanoid;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPARENA_API UWeaponHolster : public USceneComponent, public IInventory, public IInteractable
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

	/* IInteractable interface */
	virtual void OnBeginInteract_Implementation(APawn* InteractingPawn) override;
	virtual void OnEndInteract_Implementation(APawn* InteractingPawn) override;
	virtual UUserWidget* OnBeginLineTraceOver_Implementation(APawn* Pawn) override;
	virtual void OnEndLineTraceOver_Implementation(APawn* Pawn) override;
	/* IInteractable interface end */

	/* Toggle the bIsInUse status. If set to true it means, that this holster is currently trying to equip or unequip a weapon. */
	UFUNCTION(BlueprintCallable, Category = WeaponHolster)
	void ToggleIsInUse();

	/* Returns true if this holster is currently equipping or unequipping a weapon. */
	UFUNCTION(BlueprintPure, Category = WeaponHolster)
	bool IsInUse() { return bIsInUse; };

	UWeaponHolster();

	UFUNCTION(BlueprintPure, Category = WeaponHolster)
	AGun* GetCarriedGun() { return _CarriedGun; };

	UFUNCTION(BlueprintCallable, Category = WeaponHolster)
	void SwapWeapon();

	int32 GetCarryPriority() { return _CarryPriority; };

protected:
	void PlayEquipAnimation();

	UFUNCTION(BlueprintCallable, Category = WeaponHolster)
	void AttachGunToHolster();

	UFUNCTION(BlueprintCallable, Category = WeaponHolster)
	void DetachGunFromHolster();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = WeaponHolster)
	void HandleEquipWeaponEvent();

protected:
	/* The gun that is currently held in this holster. */
	AGun* _CarriedGun;	

	/* The gun that will be added to this holster shortly. */
	AGun* _NewGun;

	AHumanoid* _Owner;

	UPROPERTY(EditDefaultsOnly, Category = WeaponHolster)
	UAnimMontage* _HolsteringAnimations;	

	/* Socket name where the carried gun will be attached to. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = WeaponHolster)
	FName _AttachPoint;

	bool bIsInUse;

	/* The weapon types that this holster can carry or not carry, depending on bIsWhiteList. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = WeaponHolster)
	TArray<EWEaponType> _WeaponTypesToCarry;
	
	/* If white list, then WeaponTypesToCarry is a white list, otherwise a blacklist. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = WeaponHolster)
	bool bIsWhiteList;

	/** 
	 * The priority of this holster for carrying a gun over other inventories if the owner wants to pick up a gun and has several inventories who could carry the weapon.
	 * Higher number = higher priority.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = WeaponHolster)
	int32 _CarryPriority;
};
