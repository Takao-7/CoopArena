// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/BAS_Interface.h"
#include "Humanoid.generated.h"


class AGun;
class IInteractable;
class UDamageType;
class AItemBase;


<<<<<<< HEAD
//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHolsterWeapon_Signature);
=======
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHolsterWeapon_Signature, AGun*, Gun);


UCLASS()
class COOPARENA_API AHumanoid : public ACharacter, public IBAS_Interface
{
	GENERATED_BODY()

<<<<<<< HEAD

public:
	virtual void Tick(float DeltaTime) override;

	AHumanoid();

protected:
	virtual void BeginPlay() override;

	/////////////////////////////////////////////////////
					/* Movement */
	/////////////////////////////////////////////////////
protected:
	/** Handles moving forward/backward */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void MoveForward(float value);
	/** Handles strafing movement, left and right */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void MoveRight(float value);

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
=======

public:
	AHumanoid();

protected:
	virtual void BeginPlay() override;

	/////////////////////////////////////////////////////
					/* Movement */
	/////////////////////////////////////////////////////
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
	void SetSprinting(bool bSprint);

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void SetCrouch(bool bSprint);

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void ToggleJump();

	/** Base turn rate, in deg/sec */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	float _BaseTurnRate;

	/** Base look up/down rate, in deg/sec */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	float _BaseLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	bool bToggleProne;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	bool bToggleSprinting;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	bool bToggleCrouching;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = Humanoid)
	bool bIsSprinting;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = Humanoid)
	bool bIsProne;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	float _MaxSprintSpeed;

	UPROPERTY(BlueprintReadOnly, Category = Humanoid)
	float _MaxWalkingSpeed;


	/////////////////////////////////////////////////////
			/* Basic Animation System Interface */
	/////////////////////////////////////////////////////
public:
	bool IsAiming_Implementation() override;
	EWEaponType GetEquippedWeaponType_Implementation() override;
	EMovementType GetMovementType_Implementation() override;
	EMovementAdditive GetMovementAdditive_Implementation() override;


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
	AItemBase* GetItemInHand() { return _ItemInHand; };

	/* Sets the item in hand to nullptr, if it's not attached to this actor. */
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = Humanoid)
	void Multicast_ClearItemInHand();

protected:
	UPROPERTY(BlueprintReadOnly, Category = Humanoid)
	AItemBase* _ItemInHand;

	UPROPERTY(BlueprintReadOnly, Category = Humanoid)
	FTransform _ItemOffset;


	/////////////////////////////////////////////////////
						/* Weapon */
	/////////////////////////////////////////////////////
public:
	UFUNCTION(BlueprintPure, Category = Humanoid)
	FName GetEquippedWeaponAttachPoint() const;

	UFUNCTION(BlueprintPure, Category = Humanoid)
	AGun* GetEquippedGun() const;
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8

	UFUNCTION(BlueprintPure, Category = Humanoid)
	bool CanFire() const;

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void SetEquippedWeapon(AGun* Weapon);

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
<<<<<<< HEAD
	void SetProne(bool bProne);

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void SetSprinting(bool bSprint);

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void SetCrouch(bool bSprint);

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void ToggleJump();
=======
	bool Set_ComponentIsBlockingFiring(bool bIsBlocking, UActorComponent* Component);

	/* Called when the character wants to holster a weapon. */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = Humanoid)
	FHolsterWeapon_Signature HolsterWeapon_Event;

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	AGun* SpawnWeapon(TSubclassOf<AGun> Class);

protected:
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	virtual void OnEquipWeapon();	

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
	UPROPERTY(EditDefaultsOnly, Category = Humanoid)
	TSubclassOf<AGun> _DefaultGun;

	/** Socket or bone name for attaching weapons when equipped */
	UPROPERTY(EditDefaultsOnly, Category = Humanoid)
	FName _EquippedWeaponAttachPoint;	
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8

	/* When the Kill() function is called and the damage type does not have any specific impulse, this value will be used instead. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
<<<<<<< HEAD
	float _BaseTurnRate;

	/** Base look up/down rate, in deg/sec */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	float _BaseLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	bool bToggleProne;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	bool bToggleSprinting;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	bool bToggleCrouching;
=======
	float _DefaultInpulsOnDeath;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	UArrowComponent* _DroppedItemSpawnPoint;
		
	/* The characters currently held weapon */
	UPROPERTY(BlueprintReadWrite, Category = Humanoid)
	AGun* _EquippedWeapon;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = Humanoid)
	bool bIsAiming;
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8

private:
	/* True if a component attached to this actor wants to prevent it from firing it's weapon. */
	UPROPERTY()
	bool bComponentBlocksFiring;

	/* The component that wants to prevent this actor from firing. */
	UPROPERTY()
	UActorComponent* _BlockingComponent;

<<<<<<< HEAD
	UPROPERTY(BlueprintReadOnly, Category = Humanoid)
	bool bIsMovingForward;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	float _MaxSprintSpeed;

	UPROPERTY(BlueprintReadOnly, Category = Humanoid)
	float _MaxWalkingSpeed;


	/////////////////////////////////////////////////////
			/* Basic Animation System Interface */
	/////////////////////////////////////////////////////
public:
	bool IsAiming_Implementation() override;
	EWEaponType GetEquippedWeaponType_Implementation() override;
	EMovementType GetMovementType_Implementation() override;
	EMovementAdditive GetMovementAdditive_Implementation() override;


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
	void GrabItem(AItemBase* ItemToGrab, bool bKeepRelativeOffset, FTransform Offset);

	/* Calculates and safes the offset between the currently held item and this character. */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	FTransform CalcAndSafeActorOffset(AActor* OtherActor);

	/* Drops an actor and activates physics on that actor. */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	AItemBase* DropItem();	

	void SetUpDefaultEquipment();			

protected:
	UPROPERTY(BlueprintReadOnly, Category = Humanoid)
	AItemBase* _ItemInHand;

	UPROPERTY(BlueprintReadOnly, Category = Humanoid)
	FTransform _ItemOffset;


	/////////////////////////////////////////////////////
						/* Weapon */
	/////////////////////////////////////////////////////
public:
	UFUNCTION(BlueprintPure, Category = Humanoid)
	FName GetEquippedWeaponAttachPoint() const;

	UFUNCTION(BlueprintPure, Category = Humanoid)
	AGun* GetEquippedGun() const;

	UFUNCTION(BlueprintPure, Category = Humanoid)
	bool CanFire() const;

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void SetEquippedWeapon(AGun* weapon);

	/* 
	 * Use this function, when a component has to prevent this actor from firing his weapon.
	 * If the gun is blocked, then only the component that blocked it can unblock it.
	 * @param bisBlocking True if the actor shouldn't be allowed to fire his weapon.
	 * @param Component The component who wants to block this actor from firing. 
	 * Must not be null or this function will not do anything.
	 * @return True if the blocking status was successfully changed. E.g. if the given component
	 * was allowed to change the value.
	 */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	bool SetComponentIsBlockingFiring(bool bIsBlocking, UActorComponent* Component);

	/* Called when the character wants to holster a weapon. */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = Humanoid)
	FHolsterWeapon_Signature HolsterWeapon_Event;

protected:
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	virtual void OnEquipWeapon();	

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

	/** The tool or weapon that the character will start the game with */
	UPROPERTY(EditDefaultsOnly, Category = Humanoid)
	TSubclassOf<AGun> _DefaultGun;

	/** Socket or bone name for attaching weapons when equipped */
	UPROPERTY(EditDefaultsOnly, Category = Humanoid)
	FName _EquippedWeaponAttachPoint;	

	/* When the Kill() function is called and the damage type does not have any specific impulse, this value will be used instead. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	float _DefaultInpulsOnDeath;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	UArrowComponent* _DroppedItemSpawnPoint;

	/* The characters currently held weapon */
	UPROPERTY(BlueprintReadWrite, Category = Humanoid)
	AGun* _EquippedWeapon;

	UPROPERTY(BlueprintReadWrite, Category = Humanoid)
	bool bIsAiming;

private:
	/* True if a component attached to this actor wants to prevent it from firing it's weapon. */
	UPROPERTY()
	bool bComponentBlocksFiring;

	/* The component that wants to prevent this actor from firing. */
	UPROPERTY()
	UActorComponent* _BlockingComponent;
=======
	/////////////////////////////////////////////////////
						/* Networking */
	/////////////////////////////////////////////////////
public:
	/* [Server] Sets _WeaponToEquip and will trigger the replication function OnWeaponEquip. */
	UFUNCTION(BlueprintCallable, Category = AHumanoid)
	void SetWeaponToEquip(AGun* Weapon);

	UFUNCTION()
	void OnWeaponEquip();

protected:
	/* The weapon that should be equipped. If set to non-null that weapon will be equipped. */
	UPROPERTY(ReplicatedUsing = OnWeaponEquip)
	AGun* _WeaponToEquip;

	UFUNCTION(Server, Unreliable, WithValidation, BlueprintCallable, Category = Humanoid)
	void Server_SetSprinting(bool bSprint);

	UFUNCTION()
	void OnRep_bIsSprining();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = Humanoid)
	void RepMaxWalkSpeed(float NewMaxWalkSpeed);
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
};