// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enums/WeaponEnums.h"
#include "ItemBase.h"
#include "Gun.generated.h"


class AHumanoid;
class AProjectile;
class USoundBase;
class AMagazine;
class AItemBase;


USTRUCT(BlueprintType)
struct FGunStats
{
	GENERATED_BODY()

	FGunStats()
	{
		FireModes.Add(EFireMode::Single);
		Cooldown = 0.1f;
		SpreadHorizontal = 0.05;
		SpreadVertical = 0.05f;
		MaxSpread = 0.4f;
		ShotsPerBurst = 3;	
		lineTraceRange = 10000.0f;
	}
	/*
	* Time, in seconds, between each shot. If this value is <= 0, then the weapon can only fire
	* in Single mode, no matter what fire modes it has.
	* However, this value should NOT be <= 0.0f because then enemies will fire to quickly in single mode
	* and player could abuse the single mode for rapid fire.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Cooldown;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SpreadHorizontal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SpreadVertical;

	/**
	 * The maximum spread both, horizontal and vertical, the weapon will have.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxSpread;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<EFireMode> FireModes;

	/* If the weapon supports Burst mode, how many shots are fired in that mode. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 ShotsPerBurst;

	/* The magazine type this gun can use. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	TSubclassOf<AMagazine> UsableMagazineClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	EWEaponType WeaponType;

	/**
	* The maximum distance the gun will cast a ray when firing to adjust the aim.
	* If there isn't any viable target in that range, the shoot will travel at a strait line
	* from the barrel in the owner's view direction.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	float lineTraceRange;
};


UCLASS()
class COOPARENA_API AGun : public AItemBase
{
	GENERATED_BODY()
	
protected:
	/* The currently loaded magazine. */
	UPROPERTY(BlueprintReadWrite, Category = Weapon)
	AMagazine* _LoadedMagazine;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FGunStats _GunStats;

	/** Name of the bone or socket for the muzzle */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FName _MuzzleAttachPoint;

	/** Sound to play each time we fire */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	USoundBase* _FireSound;

	/** Sound to play each time we reload */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	USoundBase* _ReloadSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	class UAnimMontage* _FireAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	UAnimMontage* _ReloadAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	class UParticleSystem* _MuzzleFlash;

	/* The spawned muzzle flash particle system */
	UPROPERTY(BlueprintReadWrite, Category = Weapon)
	class UParticleSystemComponent* _SpawnedMuzzleFlashComponent;

	/* The owner's animation instance */
	UPROPERTY(BlueprintReadWrite, Category = Weapon)
	class UAnimInstance* _AnimInstance;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = Weapon)
	class UCameraComponent* _ZoomCamera;

	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	bool _bCanShoot;	

	/* The pawn that currently owns and carries this weapon */
	UPROPERTY(BlueprintReadWrite, Category = Weapon)
	AHumanoid* _MyOwner;

	UPROPERTY(BlueprintReadWrite, Category = Weapon)
	EFireMode _CurrentFireMode;

	/* How many shots are fired already in the current salvo. Used for the increased spread during firing. */
	UPROPERTY(BlueprintReadWrite, Category = Weapon)
	int32 _SalvoCount;

	/* How many shots are fired already in the current burst (see: burst mode). */
	UPROPERTY(BlueprintReadWrite, Category = Weapon)
	int32 _BurstCount;

	FTimerHandle _WeaponCooldownTH;

	/* Used to set the new fire mode, when the player changes the fire mode. */
	int32 _CurrentFireModePointer;

	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	EWeaponState _CurrentGunState;

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

	UFUNCTION(BlueprintCallable, Category = Weapon)
	void AttachMeshToPawn();

	UFUNCTION(BlueprintCallable, Category = Weapon)
	void DetachMeshFromPawn();

	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetOwningPawn(AHumanoid* NewOwner);

	/* Checks if the weapon is able to fire in an automatic mode (= holding Fire button results in continuous fire) */
	UFUNCTION(BlueprintPure, Category = Weapon)
	bool CanRapidFire() const;

	UFUNCTION(BlueprintCallable, Category = Weapon)
	void FinishReloadWeapon();

	virtual void BeginPlay() override;

	void SpawnNewMagazine();

	bool GetAmmoFromInventory();

	/* Checks if this gun's owner has a suitable magazine in his inventory. */
	bool CheckIfOwnerHasMagazine();

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

	FVector ApplyWeaponSpread(FVector SpawnDirection);

	UFUNCTION(BlueprintCallable, Category = Weapon)
	void OnStopFire();

	UFUNCTION(BlueprintPure, Category = Weapon)
	float GetCooldownTime() const;

	UFUNCTION(BlueprintPure, Category = Weapon)
	EWEaponType GetWeaponType() { return _GunStats.WeaponType; }

	/**
	* Reloads the weapon.
	*/
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void ReloadWeapon();

	UFUNCTION(BlueprintCallable, Category = Weapon)
	void ToggleFireMode();

	UFUNCTION(BlueprintPure, Category = Weapon)
	UMeshComponent* GetMesh() const;

	UFUNCTION(BlueprintPure, Category = Weapon)
	UCameraComponent* GetZoomCamera() const;
};
