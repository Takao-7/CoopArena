// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/StorageComponent.h"
#include "Structs/WeaponAttachPoint.h"
#include "Gun.h"
#include "Components/MeshComponent.h"
#include "InventoryComponent.generated.h"


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


	/////////////////////////////////////////////////////
						/* Networking */
	/////////////////////////////////////////////////////
private:
	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void OnOwnerHolsterWeapon_Server(AGun* GunToHolster, int32 AttachPointIndex);

	UFUNCTION(NetMulticast, Reliable, Category = "Inventory")
	void PlayHolsteringAnimation_Multicast(UAnimMontage* HolsterAnimationToPlay);

	UFUNCTION(NetMulticast, Reliable, Category = "Inventory")
	void UnequipAndAttachWeapon_Multicast(int32 AttachPointIndex, AGun* Gun);

	UFUNCTION(NetMulticast, Reliable, Category = "Inventory")
	void DetachAndEquipWeapon_Multicast(int32 AttachPointIndex);
};