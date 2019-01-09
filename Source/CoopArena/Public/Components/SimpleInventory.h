// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Magazine.h"
#include "Weapons/Gun.h"
#include "Components/ActorComponent.h"
#include "Structs/WeaponAttachPoint.h"
#include "Structs/ItemStructs.h"
#include "SimpleInventory.generated.h"


class AHumanoid;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class COOPARENA_API USimpleInventory : public UActorComponent
{
	GENERATED_BODY()


	/////////////////////////////////////////////////////
				/* Parameters & variables */
	/////////////////////////////////////////////////////
protected:
	/**
	 * The maximum number of magazines, for each magazine type, that this inventory can hold 
	 * Set to -1 for an unlimited amount of magazines for that type.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory", meta = (DisplayName = "Maximum number of magazines"))
	TMap<TSubclassOf<AMagazine>, int32> _MaxNumberOfMagazines;

	/**
	* The default maximum number of magazines for each magazine type.
	* Will only be used if a specific magazines type is not set in @see m_MaxNumberOfMagaziens.
	* Set to -1 for unlimited capacity.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Inventory", meta = (DisplayName = "Default maximum number of magazines"))
	int32 _DefaultMaxNumberOfMagazines;

	/**
	* The types and number of magazines that this inventory should spawn with.
	* Set the number to -1 for unlimited number of magazines of that type.
	* If set to -1, then 5 magazines will be dropped, when our owner dies or get destroyed.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Inventory", meta = (DisplayName = "Magazines to spawn with"))
	TMap<TSubclassOf<AMagazine>, int32> _MagazinesToSpawnWith;

	/* Magazines and their count that are currently in this inventory */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Inventory", meta = (DisplayName = "Stored magazines"))
	TArray<FMagazineStack> _StoredMagazines;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (DisplayName = "Weapon attach points"))
	TArray<FWeaponAttachPoint> _WeaponAttachPoints;

	/* Should we drop all stored magazines when our owner dies? Requires that it has a UHealthComponent! */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (DisplayName = "Drop inventory on death"))
	bool _bDropInventoryOnDeath;

	/**
	 * Should we drop all stored magazines when our owner is destroyed?
	 * If bDropInventoryOnDeath is set to true and the owner has a UHealthComponent, then this does nothing.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (DisplayName = "Drop inventory on destroy"))
	bool _bDropInventoryOnDestroy;

private:
	AHumanoid* _Owner;

	UPROPERTY(Replicated)
	int32 _AttachPointIndex;

	/* This index is used, when we holster a weapon in one slot and get the weapon from the other (this) slot. */
	UPROPERTY(Replicated)
	int32 _SecondAttachPointIndex;


	/////////////////////////////////////////////////////
						/* Getter */
	/////////////////////////////////////////////////////
public:
	/* Gets the maximum number of magazines for the given magazine type that can be stored in this inventory */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetMaxMagazineCountForType(TSubclassOf<AMagazine> MagazineType) const;

	/* Returns the number of magazines, from the given type, that are currently in this inventory */
	UFUNCTION(BlueprintCallable, Category = "Invetory")
	int32 GetNumberOfMagazinesForType(TSubclassOf<AMagazine> MagazineType) const;


	/////////////////////////////////////////////////////
					/* Check functions */
	/////////////////////////////////////////////////////
public:
	/**
	* Has this inventory enough space to store the given magazine(s)?
	* @param MagazineType The given magazine to look for
	* @param Out_FreeSpace How many magazines we can actually store
	* @param NumMagazinesToStore How many of the given magazine we want to check for
	* @return True if we have enough space to store the given magazines
	*/
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasSpaceForMagazine(TSubclassOf<AMagazine> MagazineType, int32& Out_FreeSpace, int32 NumMagazinesToStore = 1) const;

	/**
	* Has this inventory enough space to store the given magazine(s)?
	* @param NumMagazinesToStore How many of the given magazine we want to check for
	* @param MagazineType The given magazine to look for
	* @return True if we have enough space to store the given magazines
	*/
	bool HasSpaceForMagazine(int32 NumMagazinesToStore, TSubclassOf<AMagazine> MagazineType) const;

	/* Has this inventory the given number of magazines? */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasMagazines(TSubclassOf<AMagazine> MagazineType, int32 NumMagazines = 1) const;


	/////////////////////////////////////////////////////
			/* Alter magazines in inventory */
	/////////////////////////////////////////////////////
public:
	/**
	 * Adds a number of magazines, from the given type, to this inventory
	 * @return True if the magazines where successfully added.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddMagazineToInventory(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToStore = 1);

private:
	/**
	 * DO NOT CALL THIS FUNCTION DIRECTLY.
	 * Server function to add magazines on the server to the inventory.
	 * Will check if there is enough room in the inventory to add the magazines.
	 * Is called by @see AddMagazineToInventory.
	 */
	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category = "Inventory")
	void AddMagazineToInventory_Server(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToStore = 1);

public:
	/**
	 * Removes a number of magazines, from the given type, from this inventory
	 * @return True if the magazines where successfully removed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool GetMagazineFromInventory(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToRemove = 1);

private:
	/**
	 * DO NOT CALL THIS FUNCTION DIRECTLY.
	 * Server function to remove magazines on the server from the inventory.
	 * Will check if there are enough magazines in the inventory.
	 * Is called by @see GetMagazineToInventory.
	 */
	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category = "Inventory")
	void GetMagazineFromInventory_Server(TSubclassOf<AMagazine> MagazineType, int32 NumMagazinesToRemove = 1);

	void DropInventoryContent();


	/////////////////////////////////////////////////////
					/* Misc functions */
	/////////////////////////////////////////////////////
public:	
	USimpleInventory();	

protected:
	virtual void BeginPlay() override;

private:
	/* [Server] Adds the magazines to this inventory that are specified in @m_MazainesToSpawnWith */
	void SetupDefaultMagazines();

	FMagazineStack* FindMagazineStack(const TSubclassOf<AMagazine>& MagazineType);
	const FMagazineStack* FindMagazineStack(const TSubclassOf<AMagazine>& MagazineType) const;

	UFUNCTION(NetMulticast, Reliable)
	void SetOwnerInteractable(bool bCanBeInteractedWith);

	UFUNCTION()
	void OnOwnerDeath(AActor* DeadActor, AController* Controller, AController* Killer);

	UFUNCTION()
	void OnOwnerDestroyed(AActor* DestroyedOwner);

	UFUNCTION()
	void OnOwnerBeeingInteractedWith(APawn* InteractingPawn, UPrimitiveComponent* HitComponent);

	/**
	 * [Server] Transfers our entire content to the interaction pawn.
	 * If we don't have any magazines left after that, our owning pawn will be set to not be interactable.
	 */
	UFUNCTION()
	void TransfereInventoryContent(APawn* InteractingPawn);

	/* Refreshes the player hud after magazines where added or removed from his inventory. */
	UFUNCTION(Client, Reliable, Category = "Inventory")
	void RefreshHud();


	/////////////////////////////////////////////////////
					/* Weapon holstering */
	/////////////////////////////////////////////////////
public:
	/* Called when a weapon holstering is in progress and the hand is over the holster. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OnWeaponHolstering();

	/* Returns the weapon at the given attach point index or nullptr if the index is not valid or there is no gun at that index.*/
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	AGun* GetGunAtAttachPoint(int32 AttachPointIndex);

	void SetAttachPointIndex(int32 Index);

private:
	/**
	* Function that handles weapon holstering (= moving a weapon from the hands or ground to a weapon holster) and equipping (= moving a gun from a holster to the hands).
	* @param Gun The gun we want to holster. If nullptr we want to equip the gun that is in the AttachPointIndex slot.
	* @param AttachPointIndex The index for @see m_weaponAttachPoints where we want to attach the given weapon to or equip from.
	* If the value is < 0 we will search all attach points for a free, valid slot for the given gun.
	*/
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void HandleOwnerHolsterWeapon(AGun* GunToHolster, int32 AttachPointIndex);

	/* Looks for a valid slot to attach the given gun. If there is no slot, then we return -1. */
	int32 FindValidSlotForGun(AGun* GunToHolster);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void HandleOwnerEquipWeaponFromInventory(AHumanoid* Humanoid, AGun* GunToEquip, int32 AttachPointIndex);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void HandleOwnerEquipWeaponFromInventory_Server(AHumanoid* Humanoid, AGun* GunToEquip, int32 AttachPointIndex);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void OnOwnerHolsterWeapon_Server(AGun* GunToHolster, int32 AttachPointIndex);

	UFUNCTION(NetMulticast, Reliable, Category = "Inventory")
	void PlayHolsteringAnimation_Multicast(UAnimMontage* HolsterAnimationToPlay, int32 AttachPointIndex, int32 SecondAttachPointIndex);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void PlayHolsteringAnimation_Server(UAnimMontage* HolsterAnimationToPlay, int32 AttachPointIndex, int32 SecondAttachPointIndex);

	UFUNCTION(NetMulticast, Reliable, Category = "Inventory")
	void UnequipAndAttachWeapon_Multicast(int32 AttachPointIndex, AGun* Gun);

	UFUNCTION(NetMulticast, Reliable, Category = "Inventory")
	void DetachAndEquipWeapon_Multicast(int32 AttachPointIndex);

	FTimerHandle _MontageFinishedTH;
};
