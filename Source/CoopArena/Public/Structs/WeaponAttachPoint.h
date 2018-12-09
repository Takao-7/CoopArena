#pragma once

#include "CoreMinimal.h"
#include "Enums/WeaponEnums.h"
#include "Gun.h"
#include "Components/MeshComponent.h"
#include "Animation/AnimMontage.h"
#include "WeaponAttachPoint.generated.h"


USTRUCT(BlueprintType)
struct FWeaponAttachPoint
{
	GENERATED_BODY()

	FWeaponAttachPoint()
	{
		bAllowedTypesAreWhiteList = true;
		slotName = "NoName";
	};

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

	/**
	 * Can the given weapon be attached to this holster?
	 * @return True if the given weapon is valid,
	 * no weapon is currently held in this holster and
	 * if the given weapon's type is allowed in this holster.
	 */
	bool CanAttachWeapon(AGun* Weapon) const
	{
		return Weapon && m_currentlyHeldWeapon == nullptr && (allowedWeaponTypes.Find(Weapon->GetWeaponType()) != INDEX_NONE);
	}

	/**
	 * Removes the attached weapon.
	 * @return The detached weapon. Nullptr if there wasn't any weapon attached to this holster.
	 */
	AGun* DetachWeapon()
	{
		AGun* gunTemp = m_currentlyHeldWeapon;
		if (gunTemp)
		{
			m_currentlyHeldWeapon = nullptr;
			gunTemp->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		}

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