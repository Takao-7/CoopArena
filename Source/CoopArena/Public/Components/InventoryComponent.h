// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/StorageComponent.h"
#include "Structs/ItemStructs.h"
#include "Gun.h"
#include "Components/MeshComponent.h"
#include "InventoryComponent.generated.h"


USTRUCT(BlueprintType)
struct FWeaponAttachPoint
{
	GENERATED_BODY()	

	FWeaponAttachPoint()
	{
		bAllowedTypesIsWhiteList = true;
		slotName = "NoName";
	}

	/**
	 * Attaches the given weapon to this holster.
	 * @param Weapon The weapon.
	 * @param Actor The actor to attach the weapon to.
	 * @return True if the weapon was successfully attached.
	 */
	bool AttachWeapon(AGun* Weapon, UMeshComponent* Mesh)
	{
		bool bCanAttach = CanAttachWeapon(Weapon);

		if (bCanAttach)
		{
			m_currentlyHeldWeapon = Weapon;
			Weapon->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, socket);
			return true;
		}
		else
		{
			return false;
		}		
	}


	bool CanAttachWeapon(AGun* Weapon)
	{
		return (Weapon == nullptr || m_currentlyHeldWeapon || allowedWeaponTypes.Find(Weapon->GetWeaponType()) == false) ? false : true;
	}

	/**
	 * Removes the attached weapon.
	 * @return The detached weapon. Nullptr if there wasn't any weapon attached to this holster.
	 */
	AGun* DetachWeapon()
	{
		AGun* gunTemp = m_currentlyHeldWeapon;
		m_currentlyHeldWeapon = nullptr;
		gunTemp->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		return gunTemp;
	}

	/**
	 * Gets the attached weapon.
	 * @return The attached weapon. Nullptr if there isn't any weapon attached to this holster.
	 */
	FORCEINLINE AGun* GetAttachedWeapon() { return m_currentlyHeldWeapon; }

	FORCEINLINE bool operator==(const AGun* otherGun) const
	{
		return otherGun == m_currentlyHeldWeapon;
	}

	/* The socket on the owner's mesh to attach the held weapon to. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName socket;

	UPROPERTY(EditDefaultsOnly)
	FString slotName;

	/* Which weapon types can / can't (@see bAllowedTypesIsWHiteList) be held in this holster. */
	UPROPERTY(EditDefaultsOnly)
	TArray<EWEaponType> allowedWeaponTypes;

	/* Is 'allowedWeaponTypes' a white- or blacklist? */
	UPROPERTY(EditDefaultsOnly)
	bool bAllowedTypesIsWhiteList;

	/* The animation to play when holstering the weapon. */
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* holsterAnimation;

private:
	UPROPERTY()
	AGun* m_currentlyHeldWeapon;
};


class AHumanoid;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class COOPARENA_API UInventoryComponent : public UStorageComponent
{
	GENERATED_BODY()


protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (DisplayName = "Weapon attach point"))
	FWeaponAttachPoint m_WeaponAttachPoint;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory", meta = (DisplayName = "Owner"))
	AHumanoid* m_Owner;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OnOwnerWeaponHolster(AGun* Gun);

public:
	/**
	 * Called when a weapon holstering is in progress and the hand is over the holster.
	 * @param bAttachToHolster True when the gun should be attached to the holster.
	 * False if the gun should be attached to the owner's hand.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OnWeaponHolstering();
};