#pragma once

#include "CoreMinimal.h"
#include "Enums/WeaponEnums.h"
#include "Weapons/Gun.h"
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
		slotName = TEXT("NoName");
	};

	/**
	 * Attaches the given weapon to this holster.
	 * @param Weapon The weapon to attach. Must not be null.
	 * @param Mesh The mesh to attach the weapon to. This should be the mesh that this attach point belongs to.
	 * @return True if the weapon was successfully attached.
	 */
	bool AttachWeapon(AGun* Weapon, UMeshComponent* Mesh)
	{
		const bool bCanAttach = CanAttachWeapon(Weapon);

		if (bCanAttach)
		{
			_CurrentlyHeldWeapon = Weapon;
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
	 * The given weapon must not be null.
	 * @return True if the given weapon's type is allowed on this holster.
	 */
	bool CanAttachWeapon(AGun* Weapon) const
	{
		check(Weapon);

		bool bWeaponTypeIsAllowed;
		if (bAllowedTypesAreWhiteList)
		{
			bWeaponTypeIsAllowed = allowedWeaponTypes.Find(Weapon->GetWeaponType()) != INDEX_NONE;
		}
		else
		{
			bWeaponTypeIsAllowed = allowedWeaponTypes.Find(Weapon->GetWeaponType()) == INDEX_NONE;
		}
		return bWeaponTypeIsAllowed;
	}

	/**
	 * Removes the attached weapon. Only call this function if there is a weapon attached to this attach point!
	 * @return The detached weapon.
	 */
	AGun* DetachWeapon()
	{
		check(_CurrentlyHeldWeapon);

		AGun* gunTemp = _CurrentlyHeldWeapon;
		_CurrentlyHeldWeapon = nullptr;
		gunTemp->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		return gunTemp;
	}

	/**
	 * Gets the attached weapon.
	 * @return The attached weapon. Nullptr if there isn't any weapon attached to this holster.
	 */
	FORCEINLINE AGun* GetAttachedWeapon() const { return _CurrentlyHeldWeapon; }

	FORCEINLINE bool operator==(const AGun* otherGun) const
	{
		return otherGun == _CurrentlyHeldWeapon;
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
	AGun* _CurrentlyHeldWeapon;
};