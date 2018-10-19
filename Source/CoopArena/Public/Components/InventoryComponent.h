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
		bAllowedTypesAreWhiteList = true;
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
		const bool bCanAttach = CanAttachWeapon(Weapon);

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


	bool CanAttachWeapon(AGun* Weapon) const
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
	FORCEINLINE AGun* GetAttachedWeapon() const { return m_currentlyHeldWeapon; }

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
	bool bAllowedTypesAreWhiteList;

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
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (DisplayName = "Weapon attach points"))
	TArray<FWeaponAttachPoint> m_WeaponAttachPoints;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory", meta = (DisplayName = "Owner"))
	AHumanoid* m_Owner;

	/* The index for @see m_weaponAttachPoints to attach to or get a gun from. */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory", meta = (DisplayName = "Attach point index"))
	int32 m_AttachPointIndex;

public:
	/* Called when a weapon holstering is in progress and the hand is over the holster. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OnWeaponHolstering();

private:
	/**
	 * Function that handles weapon holstering (= moving a weapon from the hands or ground to a weapon holster) and equipping (= moving a gun from a holster to the hands).
	 * @param Gun The gun we want to holster. If nullptr we want to equip the gun that is in the AttachPointIndex slot.
	 * @param AttachPointIndex The index for @see m_weaponAttachPoints where we want to attach the given weapon to or equip from.
	 * If the value is < 0 we will search all attach points for a free, valid slot for the given gun.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OnOwnerHolsterWeapon(AGun* GunToHolster, int32 AttachPointIndex);
};