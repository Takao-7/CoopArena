// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


class AHumanoid;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeath_Signature, AActor*, DeadActor, AController*, Controller, AController*, Killer);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
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
	void Kill(AController* Killer);

	/* This event will be called, when this component's owner dies. */
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeath_Signature OnDeath;

private:
	UFUNCTION(NetMulticast, Reliable)
	void OnDeathEvent_Multicast(AActor* DeadActor, AController* Controller, AController* Killer);

protected:
	UPROPERTY(EditDefaultsOnly, Category = Health, meta = (DisplayName = "Max health"))
	float _MaxHealth;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = Health, meta = (DisplayName = "Current health"))
	float _CurrentHealth;

	bool bAlreadyDied;

	virtual void BeginPlay() override;

	/**
	* Called when the actor dies.
	* Does the following:
	* - Drops currentWeapon.
	* - Activates physics on the mesh.
	* - Deactivates the capsule component's collision
	* - Dispossesses the controller.
	*/
	UFUNCTION(BlueprintCallable, Category = Health)
	void HandleDeath(AActor* Owner, AController* Controller, AController* Killer);

	UFUNCTION(NetMulticast, Reliable)
	void HandleDeath_Multicast();

	void SetPhysicsOnMesh();

	AHumanoid* GetOwnerAsHumanoid();

	UFUNCTION(BlueprintCallable, Category = Health)
	void DeactivateCollisionCapsuleComponent();
	
private:
	UFUNCTION()
	void HandlePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);
};
