// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "Humanoid.generated.h"


class AGun;
class IInteractable;
class UDamageType;
class AItemBase;
class UHealthComponent;
class UBasicAnimationSystemComponent;
class USimpleInventory;
class URespawnComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHolsterWeapon_Signature, AGun*, Gun, int32, AttachPointIndex);


UCLASS()
class COOPARENA_API AHumanoid : public ACharacter
{
	GENERATED_BODY()


public:
	AHumanoid();

	UFUNCTION(BlueprintCallable, Category = "Humanoid")
	const FString& GetTeamName() const { return m_TeamName; };

	UFUNCTION(BlueprintCallable, Category = "Humanoid")
	void SetTeamName(FString NewTeamName) { m_TeamName = NewTeamName; };

	virtual void PossessedBy(AController* NewController) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Humanoid", meta = (DisplayName = "Team name"))
	FString m_TeamName;


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

	/* Increments the character's velocity by the given value. Will clamp the new velocity to the allowed range. */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void IncrementVelocity(float Increment);

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = Humanoid)
	void IncrementVelocity_Server(float Increment);

protected:
	/** Handles moving forward/backward */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	virtual void MoveForward(float value);

	/** Handles strafing movement, left and right */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	virtual void MoveRight(float value);

	/**
	* Called via input to turn at a given rate.
	* @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void TurnAtRate(float value);

	/**
	* Called via input to turn look up/down at a given rate.
	* @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void LookUpAtRate(float value);

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void SetProne(bool bProne);

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void SetSprinting(bool bWantsToSprint);

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void SetCrouch(bool bCrouch);

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void ToggleJump();

	/** Base turn rate, in deg/sec */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid, meta = (DisplayName = "Base turn rate"))
	float m_BaseTurnRate;

	/** Base look up/down rate, in deg/sec */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid, meta = (DisplayName = "Base Look up rate"))
	float m_BaseLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid, meta = (DisplayName = "Toggle prone"))
	bool m_bToggleProne;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid, meta = (DisplayName = "Toggle sprinting"))
	bool m_bToggleSprinting;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid, meta = (DisplayName = "Toggle crouching"))
	bool m_bToggleCrouching;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = Humanoid)
	bool m_bIsSprinting;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = Humanoid)
	bool m_bIsProne;

	/* The maximum speed at which this character can move forwards. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid, meta = (DisplayName = "Max forward speed"))
	float m_MaxForwardSpeed;

	/* If the character's speed is greater than this, he is sprinting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid, meta = (DisplayName = "Sprinting speed threshold"))
	float m_SprintingSpeedThreshold;

	/* The maximum velocity, in cm/s, at which the character can crouch (forward and backward).  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid, meta = (DisplayName = "Max crouching speed"))
	float m_MaxCrouchingSpeed;

	/* The maximum speed at which the character can move backwards. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid, meta = (DisplayName = "Max backwards speed"))
	float m_MaxBackwardsSpeed;

	/* The character's speed before he started sprinting. */
	UPROPERTY(Replicated)
	float m_SpeedBeforeSprinting;


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
	FHolsterWeapon_Signature HolsterWeapon_Event;

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	AGun* SpawnWeapon(TSubclassOf<AGun> Class);

	UFUNCTION(BlueprintPure, Category = Humanoid)
	bool IsAiming() const;

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
	AGun* m_EquippedWeapon;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = Humanoid)
	bool m_bIsAiming;

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
	AGun* m_WeaponToEquip;

	UFUNCTION(Server, WithValidation, Reliable)
	void EquipWeapon_Server(AGun* Gun);

	/* Handles the actual weapon un-equipping. */
	UFUNCTION(NetMulticast, Reliable)
	void HandleWeaponUnEquip_Multicast(bool bDropGun);

	UFUNCTION()
	void OnRep_bIsSprining();

	UFUNCTION(Server, WithValidation, Reliable)
	void UnequipWeapon_Server(bool bDropGun);
};