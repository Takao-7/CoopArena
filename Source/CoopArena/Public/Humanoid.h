// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/BAS_Interface.h"
#include "Humanoid.generated.h"


class AGun;
class IInteractable;
class UDamageType;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipWeapon_Signature);


UCLASS()
class COOPARENA_API AHumanoid : public ACharacter, public IBAS_Interface
{
	GENERATED_BODY()

public:	
	/**
	 * Checks if the character is alive.
	 * @return true if alive, otherwise false
	 */
	UFUNCTION(BlueprintPure, Category = Humanoid)
	bool IsAlive() const;

	UFUNCTION(BlueprintPure, Category = Humanoid)
	bool CanFire() const;

	UFUNCTION(BlueprintPure, Category = Humanoid)
	FName GetWeaponAttachPoint() const;

	void SetUpDefaultEquipment();

	/* Kills this character */
	void Kill();
	void Kill(const UDamageType* DamageType, FVector Direction, FVector HitLocation);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Sets default values for this character's properties
	AHumanoid();

	UFUNCTION(BlueprintPure, Category = Humanoid)
	AGun* GetEquippedGun() const;

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void SetEquippedWeapon(AGun* weapon);

	/* Begin BAS Interface */

	bool IsAiming_Implementation() override;
	EWEaponType GetEquippedWeaponType_Implementation() override;
	EMovementType GetMovementType_Implementation() override;
	EMovementAdditive GetMovementAdditive_Implementation() override;

	/* End BAS Interface */

protected:
	UFUNCTION()
	void ImpulseOnDeath(FVector Direction, FVector HitLocation);

	virtual void ToggleProne();

	virtual void OnEquipWeapon();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;	

	/* Fire the currently equipped weapon */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	virtual void FireEquippedWeapon();

	/* Stops firing the currently equipped weapon */
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	virtual void StopFireEquippedWeapon();
	
	/**
	* Called when the actor dies.
	* Does the following:
	* - Un-Equips (=drops) currentWeapon.
	* - Activates physics on the mesh.
	* - Deactivates the capsule component's collision
	* - Dispossesses the controller.
	* - Adds an impulse on the last hit's location in the opposite direction of the last hit.
	* So the actor is pushed away from the last hit.
	* @param applyImpulseOnDeath If set to true an impulse will be applied at the hit's location.
	* @param hitInfo The FHitResult struct of the shot that killed the pawn.
	* @param direction The direction from which the last shot came.
	*/
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	virtual void OnDeath();

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	virtual void DeactivateCollisionCapsuleComponent();

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
	virtual void TurnAtRate(float value);

	/**
	* Called via input to turn look up/down at a given rate.
	* @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	virtual void LookUpAtRate(float value);

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	virtual void ToggleCrouch();

	UFUNCTION(BlueprintCallable, Category = PlayerCharacter)
	virtual void ToggleAiming();

	UFUNCTION(BlueprintCallable, Category = PlayerCharacter)
	void StopSprinting();

	UFUNCTION(BlueprintCallable, Category = PlayerCharacter)
	void StartSprinting();

	UFUNCTION(BlueprintCallable, Category = PlayerCharacter)
	void SetSprinting(bool bSprint);

	UFUNCTION(BlueprintCallable, Category = PlayerCharacter)
	void ToggleJump();

	UFUNCTION(BlueprintCallable, Category = PlayerCharacter)
	void ReloadWeapon();

	UFUNCTION(BlueprintCallable, Category = PlayerCharacter)
	void ChangeWeaponFireMode();

protected:
	/** The tool or weapon that the character will start the game with */
	UPROPERTY(EditDefaultsOnly, Category = Humanoid)
	TSubclassOf<AGun> _DefaultGun;

	/** Socket or bone name for attaching weapons when equipped */
	UPROPERTY(EditDefaultsOnly, Category = Humanoid)
	FName _WeaponAttachPoint;

	/** Base turn rate, in deg/sec */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	float _BaseTurnRate;

	/** Base look up/down rate, in deg/sec */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	float _BaseLookUpRate;

	/* When the Kill() function is called and the damage type does not have any specific impulse, this value will be used instead. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	float _DefaultInpulsOnDeath;

	/* The characters currently held weapon */
	UPROPERTY(BlueprintReadWrite, Category = Humanoid)
	AGun* _EquippedWeapon;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	UArrowComponent* _DroppedItemSpawnPoint;

	UPROPERTY(BlueprintReadWrite, Category = Humanoid)
	bool bIsAiming;

	UPROPERTY(BlueprintReadOnly, Category = Humanoid)
	bool bIsSprinting;

	UPROPERTY(BlueprintReadOnly, Category = Humanoid)
	bool bIsProne;

	UPROPERTY(BlueprintReadOnly, Category = Humanoid)
	bool bAlreadyDied;

	UPROPERTY(BlueprintReadOnly, Category = Humanoid)
	bool bIsMovingForward;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Humanoid)
	float _SprintSpeedIncrease;

	UPROPERTY(BlueprintReadOnly, Category = Humanoid)
	float _DefaultMaxWalkingSpeed;

	UPROPERTY(BlueprintReadOnly, Category = Humanoid)
	bool bIsJumping;

public:
	/* Called when the character wants to equip a weapon. */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = Humanoid)
	FEquipWeapon_Signature EquipWeapon_Event;
};