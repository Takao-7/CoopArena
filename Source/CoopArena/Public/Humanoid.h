// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "Interfaces/Interactable.h"
#include "Enums/WeaponEnums.h"
#include "Enums/BAS_Enums.h"
#include "Humanoid.generated.h"


class AGun;
class IInteractable;
class UDamageType;
class AItemBase;
class UHealthComponent;
class UBasicAnimationSystemComponent;
class USimpleInventory;
class URespawnComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBeginInteract_Signature, APawn*, InteractingPawn, UPrimitiveComponent*, HitComponent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBeginLineTraceOver_Signature, APawn*, Pawn, UPrimitiveComponent*, HitComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndLineTraceOver_Signature, APawn*, Pawn);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReloadFinished_Signature, AHumanoid*, Character, AGun*, Gun);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFireWeapon_Signature, AHumanoid*, Character, AGun*, Gun);

/* This event is called when we have equipped a weapon. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponEquipped_Signature, AHumanoid*, Character, AGun*, Gun);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFireModeChange_Signature, AHumanoid*, Character, EFireMode, NewFireMode);

/**
 * This event will be called when we want to holster the given weapon at the attach point.
 * The inventory should pick this up and handle the actual holstering.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHolsterWeapon_Signature, AGun*, Gun, int32, AttachPointIndex);

/**
 * This event will be called when we want to equip a weapon from our inventory.
 * The inventory should pick this up and handle the actual equipping and holstering.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEquipWeaponFromInventory_Signature, AHumanoid*, Humanoid, AGun*, Gun, int32, AttachPointIndex);


UCLASS(abstract)
class COOPARENA_API AHumanoid : public ACharacter, public IInteractable
{
	GENERATED_BODY()


public:
	AHumanoid();

	UFUNCTION(BlueprintCallable, Category = "Humanoid")
	const FString& GetTeamName() const { return _TeamName; };

	UFUNCTION(BlueprintCallable, Category = "Humanoid")
	void SetTeamName(FString NewTeamName) { _TeamName = NewTeamName; };

	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(BlueprintPure, Category = "Humanoid")
	bool IsAlive() const;

	/**
	 * Revives this character at it's death location.
	 * @param bSpawnDefaultEquippment Should the revived character revive with the default equipment or
	 * keep his old inventory?
	 */
	void Revive(bool bSpawnDefaultEquipment = false);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Humanoid", meta = (DisplayName = "Team name"))
	FString _TeamName;


	/////////////////////////////////////////////////////
				/* Interactable interface */
	/////////////////////////////////////////////////////
public:
	virtual void OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent) override;
	//virtual void OnEndInteract_Implementation(APawn* InteractingPawn) override;
	virtual UUserWidget* OnBeginLineTraceOver_Implementation(APawn* Pawn, UPrimitiveComponent* HitComponent) override;
	virtual void OnEndLineTraceOver_Implementation(APawn* Pawn) override;
	virtual void SetCanBeInteractedWith_Implementation(bool bCanbeInteractedWith) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Humanoid|Interactable", meta = (DisplayName = "Can be interacted with"))
	bool _bCanBeInteractedWith;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Humanoid|Interactable")
	UUserWidget* _LineTraceOverUserWidget;

	UPROPERTY(BlueprintAssignable, Category = "Humanoid|Interactable")
	FOnBeginInteract_Signature OnBeginInteract_Event;

	UPROPERTY(BlueprintAssignable, Category = "Humanoid|Interactable")
	FOnBeginLineTraceOver_Signature OnBeginLineTraceOver_Event;

	UPROPERTY(BlueprintAssignable, Category = "Humanoid|Interactable")
	FOnEndLineTraceOver_Signature OnEndLineTraceOver_Event;


	/////////////////////////////////////////////////////
					/* Components */
	/////////////////////////////////////////////////////
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (DisplayName = "Health"))
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (DisplayName = "Basic Animation System"))
	UBasicAnimationSystemComponent* BASComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (DisplayName = "Inventory"))
	USimpleInventory* Inventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (DisplayName = "Respawn"))
	URespawnComponent* RespawnComponent;


	/////////////////////////////////////////////////////
					/* Movement */
	/////////////////////////////////////////////////////
public:
	/* Sets the character's velocity to the given value. Will clamp the new velocity to the allowed range. */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void SetVelocity(float NewVelocity);
	
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = Humanoid)
	void SetVelocity_Server(float NewVelocity);

protected:
	/** Handles moving forward/backward */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	virtual void MoveForward(float value);

	/** Handles strafing movement, left and right */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	virtual void MoveRight(float value);

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void SetProne(bool bProne);

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	virtual void SetSprinting(bool bWantsToSprint);

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void SetWalking(bool bWantsToWalk);

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void SetCrouch(bool bCrouch);

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void ToggleJump();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Humanoid|Movement")
	bool _bIsProne;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Humanoid|Movement", meta = (DisplayName = "Sprinting speed"))
	float _SprintingSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Humanoid|Movement", meta = (DisplayName = "Jogging speed"))
	float _JoggingSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Humanoid|Movement", meta = (DisplayName = "Walking speed"))
	float _WalkingSpeed;

	/* The maximum velocity, in cm/s, at which the character can crouch (forward and backward).  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Humanoid|Movement", meta = (DisplayName = "Max crouching speed"))
	float _MaxCrouchingSpeed;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Humanoid|Movement")
	EGait _Gait;


	/////////////////////////////////////////////////////
					/* Interaction */
	/////////////////////////////////////////////////////
public:
	/* 
	 * Attaches an item to a hand.
	 * @param ItemToGrab Must not be null.
	 * @param bKeepRelativeOffset Weather or not to keep the relative offset between the ActorToGrab and this actor.
	 */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void GrabItem(AItemBase* ItemToGrab, bool bKeepRelativeOffset);

	/* Calculates and safes the offset between the currently held item and this character. */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	FTransform CalcAndSafeActorOffset(AActor* OtherActor);

	/* Drops an actor and activates physics on that actor. */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	AItemBase* DropItem();	

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void SetUpDefaultEquipment();

	/* Returns the item offset in local (relative to the 'HandLeft' socket, or world space. */
	UFUNCTION()
	FTransform GetItemOffset(bool bInLocalSpace = true);

	/* Returns the item currently held in the hand. */
	UFUNCTION()
	AItemBase* GetItemInHand() { return m_ItemInHand; };

	/* Sets the item in hand to nullptr, if it's not attached to this actor. */
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = Humanoid)
	void Multicast_ClearItemInHand();

protected:
	UPROPERTY(BlueprintReadOnly, Category = Humanoid)
	AItemBase* m_ItemInHand;

	UPROPERTY(BlueprintReadOnly, Category = Humanoid)
	FTransform m_ItemOffset;


	/////////////////////////////////////////////////////
						/* Weapon */
	/////////////////////////////////////////////////////
public:
	/**
	 * Equips the given weapon.
	 * @param WeaponToEquip The weapon to equip.
	 * @param bRequestNetMulticast If true, then the weapon will be equipped on all clients. If false, only on the client that calls this function.
	 */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void EquipWeapon(AGun* WeaponToEquip, bool bRequestNetMulticast = true);

	/**
	 * Un-equips the currently held weapon.
	 * @param bDropGun If true, the gun will be dropped (Simulate physics and can be interacted with).
	 * Set to false if the weapon goes to a holster.
	 */ 
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void UnequipWeapon(bool bDropGun, bool bRequestNetMulticast = true);

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void SetEquippedWeaponFireMode(EFireMode NewFireMode);

	UFUNCTION(BlueprintPure, Category = Humanoid)
	FName GetEquippedWeaponAttachPoint() const;

	UFUNCTION(BlueprintPure, Category = Humanoid)
	AGun* GetEquippedGun() const;

	UFUNCTION(BlueprintPure, Category = Humanoid)
	bool CanFire() const;

	/* 
	 * Use this function, when a component has to prevent this actor from firing his weapon.
	 * If the gun is blocked, then only the component that blocked it can unblock it.
	 * @param bisBlocking True if the actor shouldn't be allowed to fire his weapon.
	 * @param Component The component who wants to block this actor from firing. 
	 * Must not be null or this function will not do anything.
	 * @return True if the blocking status was successfully changed. E.g. if the given component
	 * was allowed to change the value. False if the status wasn't changed.
	 */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	bool SetComponentIsBlockingFiring(bool bIsBlocking, UActorComponent* Component);

	/* Called when the character wants to holster a weapon. */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = Humanoid)
	FOnHolsterWeapon_Signature HolsterWeapon_Event;

	/* Called when the character wants to equip a weapon from his inventory. */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = Humanoid)
	FOnEquipWeaponFromInventory_Signature EquipWeaponFromInventory_Event;

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	AGun* SpawnWeapon(TSubclassOf<AGun> Class);

	UFUNCTION(BlueprintPure, Category = Humanoid)
	bool IsAiming() const;

	/**
	 * Gets the number of rounds left in the equipped weapon. 
	 * @return The number of rounds left. If no weapon is equipped or the weapon does not have
	 * a magazine, then 0.
	 * A value of -1 means that the magazine has infinite ammo.
	 */
	UFUNCTION(BlueprintPure, Category = Humanoid)
	int32 GetNumRoundsLeft();

	/* This event is called, when we have finished reloading our weapon. */
	UPROPERTY(BlueprintAssignable, Category = "Humanoid")
	FOnReloadFinished_Signature OnReloadFinished;

	/* This event is called each time we fire a weapon. */
	UPROPERTY(BlueprintAssignable, Category = "Humanoid")
	FOnFireWeapon_Signature OnWeaponFire;

	UPROPERTY(BlueprintAssignable, Category = "Humanoid")
	FOnFireModeChange_Signature OnFireModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Humanoid")
	FOnWeaponEquipped_Signature OnWeaponEquipped;

protected:
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	virtual void OnHolsterWeapon();

	/* Fire the currently equipped weapon */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	virtual void FireEquippedWeapon();

	/* Stops firing the currently equipped weapon */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	virtual void StopFireEquippedWeapon();

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	virtual void ToggleAiming();

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void ReloadWeapon();

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void ChangeWeaponFireMode();

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void GetWeaponSpawnTransform(FTransform& OutTransform);

	/** The tool or weapon that the character will start the game with */
	UPROPERTY(EditDefaultsOnly, Category = Humanoid, meta = (DisplayName = "Default gun"))
	TSubclassOf<AGun> m_DefaultGun;

	/** Socket or bone name for attaching weapons when equipped */
	UPROPERTY(EditDefaultsOnly, Category = Humanoid, meta = (DisplayName = "Equipped weapon attach point"))
	FName m_EquippedWeaponAttachPoint;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	UArrowComponent* m_DroppedItemSpawnPoint;
		
	/* The characters currently held weapon */
	UPROPERTY(BlueprintReadWrite, Category = Humanoid, meta = (DisplayName = "Equipped weapon"))
	AGun* _EquippedWeapon;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = Humanoid)
	bool _bIsAiming;

private:
	/* True if a component attached to this actor wants to prevent it from firing it's weapon. */
	UPROPERTY()
	bool m_bComponentBlocksFiring;

	/* The component that wants to prevent this actor from firing. */
	UPROPERTY()
	UActorComponent* m_BlockingComponent;

	/* Handles the actual weapon un-equipping. */
	void HandleWeaponUnEquip(bool bDropGun);

	/* Handles the actual weapon equipping. */
	UFUNCTION()
	void HandleWeaponEquip();


	/////////////////////////////////////////////////////
						/* Networking */
	/////////////////////////////////////////////////////
protected:
	/* Sets m_WeaponToEquip and will trigger the replication function OnWeaponEquip. */
	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category = Humanoid)
	void SetWeaponToEquip_Server(AGun* Weapon);

	UFUNCTION(Server, Unreliable, WithValidation, BlueprintCallable, Category = Humanoid)
	void SetSprinting_Server(bool bWantsToSprint);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = Humanoid)
	void RepMaxWalkSpeed(float NewMaxWalkSpeed);

private:
	/* The weapon that should be equipped. */
	UPROPERTY(ReplicatedUsing = HandleWeaponEquip)
	AGun* _WeaponToEquip;

	UFUNCTION(Server, WithValidation, Reliable)
	void EquipWeapon_Server(AGun* Gun);

	/* Handles the actual weapon un-equipping. */
	UFUNCTION(NetMulticast, Reliable)
	void HandleWeaponUnEquip_Multicast(bool bDropGun);

	UFUNCTION(Server, WithValidation, Reliable)
	void UnequipWeapon_Server(bool bDropGun);
};