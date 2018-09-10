// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


class AHumanoid;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPARENA_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();

	/**
	 * Checks if the character is alive.
	 * @return true if alive, otherwise false
	 */
	UFUNCTION(BlueprintPure, Category = Health)
	FORCEINLINE bool IsAlive() const;

	/* Kills the attached actor */
	UFUNCTION(BlueprintCallable, Category = Health)
	void Kill();

protected:
<<<<<<< HEAD
	UPROPERTY(EditDefaultsOnly, Category = Health)
	float MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category = Health)
	float CurrentHealth;

	// Called when the game starts
=======
	UPROPERTY(EditDefaultsOnly, Category = Health, meta = (DisplayName = "Max health"))
	float _MaxHealth;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = Health, meta = (DisplayName = "Current health"))
	float _CurrentHealth;

	UPROPERTY()
	bool bAlreadyDied;

	UPROPERTY()
	AHumanoid* _compOwner;

>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
	virtual void BeginPlay() override;

	/**
	* Called when the actor dies.
	* Does the following:
	* - Drops currentWeapon.
	* - Activates physics on the mesh.
	* - Deactivates the capsule component's collision
	* - Dispossesses the controller.
	* - Adds an impulse on the last hit's location in the opposite direction of the last hit.
	* So the actor is pushed away from the last hit.
	* @param applyImpulseOnDeath If set to true an impulse will be applied at the hit's location.
	* @param hitInfo The FHitResult struct of the shot that killed the pawn.
	* @param direction The direction from which the last shot came.
	*/
	UFUNCTION(BlueprintCallable, Category = Health)
	void OnDeath();

<<<<<<< HEAD
=======
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDeath();

>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
	void SetPhysicsOnMesh();

	AHumanoid* GetOwnerAsHumanoid();

	UFUNCTION(BlueprintCallable, Category = Health)
	void DeactivateCollisionCapsuleComponent();
	
private:
	UFUNCTION()
	void HandlePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);
<<<<<<< HEAD

	bool bAlreadyDied;

	AHumanoid* _compOwner;
=======
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
};
