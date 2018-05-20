// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "Gun.generated.h"


class AHumanoid;
class AProjectile;


UENUM(BlueprintType)
enum class EFireMode : uint8
{
	 Single,
	 Burst,
	 Auto
};


UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Idle,
	Firing,
	Reloading,
	Equipping,
	Blocked
};


UENUM(BlueprintType)
enum class EWEaponType : uint8
{
	None,
	Pistol,
	Rifle,
	Shotgun,
	Launcher
};


USTRUCT(BlueprintType)
struct FGunStats
{
	GENERATED_BODY()

	FGunStats()
	{
		FireModes.Add(EFireMode::Single);
		Cooldown = 0.1f;
		MagazineSize = 30;		
	}
	/*
	* Time, in seconds, between each shot. If this value is <= 0, then the weapon can only fire
	* in Single mode, no matter what fire modes it has.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Cooldown;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MagazineSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SpreadHorizontal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SpreadVertical;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<EFireMode> FireModes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	TSubclassOf<AProjectile> ProjectileToSpawn;

	/* How many shots are left in the magazine */
	UPROPERTY(BlueprintReadWrite, Category = Weapon)
	int32 ShotsLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	EWEaponType WeaponType;
};


UCLASS()
class COOPARENA_API AGun : public AItemBase
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FGunStats _WeaponStats;

	/** Name of the bone or socket for the muzzle */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FName _MuzzleAttachPoint;

	/** Sound to play each time we fire */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	class USoundBase* _FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	class UAnimMontage* _FireAnimation;

	UPROPERTY(BlueprintReadOnly, BlueprintReadOnly, Category = Weapon)
	class UAnimInstance* _AnimInstance;

	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	bool _bCanShoot;	

	/* The pawn that currently owns and carries this weapon */
	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	AHumanoid* _MyOwner;

	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	EWeaponState _CurrentState;	

	/**
	* The maximum distance the gun will cast a ray when firing to adjust the aim.
	* If there isn't any viable target in that range, the shoot will travel at a strait line
	* from the barrel in the owner's view direction.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	float _lineTraceRange;

	FTimerHandle _WeaponCooldownTimer;
	
	EFireMode _CurrentFireMode;

protected:
	/**
	* Adjusts the aim based on lineTraceRange.
	* Makes a line trace from startLocation forwards to StartLocation * lineTraceRange.
	* The line trace is by channel ECollisionChannel::ECC_Camera.
	* If a viable target is hit, then the return rotator points from the muzzle location towards the hit point.
	* If not viable target was hit, direction.Rotation() is returned.
	*
	* @param startLocation The start location where the line trace starts.
	* @param direction The direction in which the line trace will go.
	* @return RotatorIf a viable target was hit, then the target location, otherwise the StartLocation.
	*/
	FVector AdjustAimRotation(FVector startLocation, FVector direction);	

	/** Function is called after the WeaponCooldownTimer is called and activated. */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void ContinousOnFire();

	UFUNCTION(BlueprintPure, Category = Weapon)
	FVector GetForwardCameraVector() const;

	void AttachMeshToPawn();
	void DetachMeshFromPawn();
	void SetOwningPawn(AHumanoid* NewOwner);

public:
	AGun();

	/* IInteractable interface */
	virtual void OnBeginInteract_Implementation(APawn* InteractingPawn) override;
	/* IInteractable interface end */

	/** Returns the number of rounds the weapon can fire each minute. */
	UFUNCTION(BlueprintPure, Category = Weapon)
	float GetRoundsPerMinute() const;
	
	/** Returns the muzzle sockets location in world space. */
	UFUNCTION(BlueprintPure, Category = Weapon)
	FVector GetMuzzleLocation() const;

	UFUNCTION(BlueprintPure, Category = Weapon)
	bool CanShoot() const;

	UFUNCTION(BlueprintCallable, Category = Weapon)
	void OnEquip(AHumanoid* NewOwner);

	/* Unequip the gun. 
	 * @param DropGun Set to false if the weapon should go to the inventory (hide mesh, no collision and can't fire),
	 * otherwise it will be dropped.
	 */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void OnUnequip(bool DropGun = false);

	UFUNCTION(BlueprintCallable, Category = Weapon)
	void OnFire();

	UFUNCTION(BlueprintPure, Category = Weapon)
	float GetCooldownTime() const;
};