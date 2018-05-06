// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Humanoid.generated.h"

class AWeapon;

UCLASS()
class COOPARENA_API AHumanoid : public ACharacter
{
	GENERATED_BODY()

public:	
	/**
	 * Checks if the character is alive.
	 * @return true if alive, otherwise false
	 */
	UFUNCTION(BlueprintPure, Category = Gameplay)
	bool IsAlive() const;

	UFUNCTION(BlueprintPure, Category = Gameplay)
	bool CanFire() const;

	UFUNCTION(BlueprintPure, Category = Gameplay)
	FName GetWeaponAttachPoint() const;

	void SetUpDefaultEquipment();

	/* Kills this character */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	void Kill();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Sets default values for this character's properties
	AHumanoid();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;	

	/* Fire the currently equipped weapon */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	void FireEquippedWeapon();

	/* Stops firing the currently equipped weapon */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	void StopFireEquippedWeapon();


	/**
	* Called when the actor dies.
	* Does the following:
	* 1) Sets bAlreadyDead to true (preventing the actor from "dying" several times).
	* 2) Uneqquips (=drops) currentWeapon.
	* 3) Activates physics on the third person mesh & hides the first person mesh.
	* 4) Deactivates the capsule component's collision
	* 5) Unpossesses the controller.
	* 5) Adds an impulse on the last hit's location in the opposite direction of the last hit.
	* So the actor is pushed away from the last hit.
	* @param applyImpulseOnDeath If set to true an impulse will be applied at the hit's location.
	* @param hitInfo The FHitResult struct of the shot that killed the pawn.
	* @param direction The direction from which the last shot came.
	*/
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	void OnDeath(const FHitResult& hitInfo, FVector direction);

	void DeactivateCollisionCapsuleComponent();

	/** Handles moving forward/backward */
	void MoveForward(float);

	/** Handles strafing movement, left and right */
	void MoveRight(float);

	/**
	* Called via input to turn at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float);

	/**
	* Called via input to turn look up/down at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void LookUpAtRate(float);

	void ToggleCrouch();

	/** The weapon that the character will start the game with */
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TSubclassOf<AWeapon> DefaultWeapon;

	/** Socket or bone name for attaching weapons when equipped */
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	FName WeaponAttachPoint;

	/** Base turn rate, in deg/sec */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Equipment)
	AWeapon* EquippedWeapon;

private:
	bool bAlreadyDied;

};