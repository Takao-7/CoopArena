// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "Gun.generated.h"


class AHumanoid;
class AProjectile;
//class UUserWidget;


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


UCLASS()
class COOPARENA_API AGun : public AItemBase
{
	GENERATED_BODY()
	
protected:
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Weapon)
	int32 _MagazineSize;

	UPROPERTY(BlueprintReadWrite, Category = Weapon)
	int32 _ShotsLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	TSubclassOf<AProjectile> _ProjectileToSpawn;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	USkeletalMeshComponent* _Mesh;

	/* The pawn that currently owns and carries this weapon */
	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	AHumanoid* _MyOwner;

	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	EWeaponState _CurrentState;

	/* 
	 * Time, in seconds, between each shot. If this value is <= 0, then the weapon can only fire
	 * in Single mode, no matter what fire modes it has. 
	 */
	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	float _Cooldown;

	/**
	* The maximum distance the gun will cast a ray when firing to adjust the aim.
	* If there isn't any viable target in that range, the shoot will travel at a strait line
	* from the barrel in the owner's view direction.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	float _lineTraceRange;

	FTimerHandle _WeaponCooldownTimer;
	float _SpreadHorizontal;
	float _SpreadVertical;
	EFireMode _CurrentFireMode;

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
	void SetOwningPawn(AActor* NewOwner);

public:
	AGun();

	/** Returns the number of rounds the weapon can fire each minute. */
	UFUNCTION(BlueprintPure, Category = Weapon)
	float GetRoundsPerMinute() const;
	
	/** Returns the muzzle sockets location in world space. */
	UFUNCTION(BlueprintPure, Category = Weapon)
	FVector GetMuzzleLocation() const;

	UFUNCTION(BlueprintPure, Category = Weapon)
	bool CanShoot() const;

	void OnEquip(AHumanoid* NewOwner);

	/* Unequip the gun. 
	 * @param DropGun Set to false if the weapon should go to the inventory (hide mesh, no collision and can't fire),
	 * otherwise it will be dropped.
	 */
	void OnUnequip(bool DropGun = false);

	void OnFire();

	float GetCooldownTime() const;
};