// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"


class AHumanoid;
class USkeletalMeshComponent;


UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Idle,
	Firing,
	Reloading,
	Equipping
};


UCLASS()
class COOPARENA_API AWeapon : public AActor
{
	GENERATED_BODY()

protected:
	/** Weapon mesh */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	USkeletalMeshComponent* Mesh;

	/** The cool down between each shot/attack in seconds */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	float Cooldown;

	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	AHumanoid* MyPawn;

	EWeaponState CurrentState;
	FTimerHandle WeaponCooldownTimer;

public:	
	AWeapon();

	UFUNCTION()
	virtual void Equip(AHumanoid* NewOwner);

	UFUNCTION()
	virtual void UnEquip();

	/** Returns the cooldown between each shot / attack */
	UFUNCTION(BlueprintPure, Category = Weapon)
	virtual float GetCooldown() const;

	/** Returns true if the weapon is able to shoot / attack at the moment */
	UFUNCTION(BlueprintPure, Category = Weapon)
	virtual bool CanShoot() const;

	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual void SetOwningPawn(AHumanoid* NewOwner);

	UFUNCTION(BlueprintPure, Category = Weapon)
	virtual USkeletalMeshComponent* GetWeaponMesh() const;

protected:
	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual void AttachMeshToPawn();

	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual void DetachMeshFromPawn();

	UFUNCTION()
	virtual void Use();
	
	UFUNCTION()
	virtual void StopUsing();
};