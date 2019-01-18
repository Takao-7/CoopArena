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
class UBoxComponent;
class UArrowComponent;
class UCameraComponent;
class UAnimMontage;
class UParticleSystem;
class UAnimInstance;


USTRUCT(BlueprintType)
struct FGunStats
{
	GENERATED_BODY()

	FGunStats()
	{
		FireModes.Add(EFireMode::Single);
		FireModes.Add(EFireMode::Burst);
		FireModes.Add(EFireMode::Auto);
		Cooldown = 0.1f;
		SpreadHorizontal = 0.25f;
		SpreadVertical = 1.0f;
		KickbackSpeed = 10.0f;
		ShotsPerBurst = 3;	
		LineTraceRange = 10000.0f;
	}
	/*
	* Time, in seconds, between each shot. If this value is <= 0, then the weapon can only fire
	* in Single mode, no matter what fire modes it has.
	* However, this value should NOT be <= 0.0f because then enemies will fire to quickly in single mode
	* and player could abuse the single mode for rapid fire.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float Cooldown;

	/* The horizontal spread cone (random between +/- SpreadHorizontal), in degree, that will be applied after each shot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float SpreadHorizontal;

	/* Vertical spread (kickback), in degree, that will be applied after each shot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float SpreadVertical;

	/* How fast the spread (kickback) is applied */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float KickbackSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TArray<EFireMode> FireModes;

	/* If the weapon supports Burst mode, how many shots are fired in that mode. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	int32 ShotsPerBurst;

	/* The magazine type this gun can use. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AMagazine> UsableMagazineClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EWEaponType WeaponType;

	/**
	* The maximum distance the gun will cast a ray when firing to adjust the aim.
	* If there isn't any viable target in that range, the shoot will travel at a strait line
	* from the barrel in the owner's view direction.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float LineTraceRange;
};


UCLASS()
class COOPARENA_API AGun : public AItemBase
{
	GENERATED_BODY()
	

	/////////////////////////////////////////////////////
					/* Parameters */
	/////////////////////////////////////////////////////
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (DisplayName = "Gun Stats"))
	FGunStats _GunStats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (DisplayName = "Zoom Camera"))
	UCameraComponent* _ZoomCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (DisplayName = "Mesh"))
	USkeletalMeshComponent* _Mesh;

	/** Name of the bone or socket for the muzzle */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (DisplayName = "Muzzle attach point"))
	FName _MuzzleAttachPoint;

	/* Name of the bone / socket where shells are ejected, when firing a bullet. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (DisplayName = "Shell ejection point"))
	FName _ShellEjectionPoint;

	/** Sound to play each time we fire */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (DisplayName = "Fire sound"))
	USoundBase* _FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (DisplayName = "Fire animation"))
	UAnimMontage* _FireAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (DisplayName = "Muzzle flash"))
	UParticleSystem* _MuzzleFlash;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (DisplayName = "Reload animation"))
	UAnimMontage* _ReloadAnimation;
	
	/**
	 * The offset to properly equip this weapon, after attaching this weapon to the carrier,
	 * we will be moved and rotated by this.
	 * The offset has to be placed at the trigger, where it is touched by the pointer finger.
	 * The X-Axis faces along the barrel, the Y-Axis to the right.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USceneComponent* _EquipOffset;

	/**
	 * The position where the left hand will be placed on the gun.
	 * This offset is used by the animation blueprint for the inverse kinematic.
	 * Place this under the weapon, where the inner palm would touch the weapon.
	 * The X-Axis faces along the barrel, the Y-Axis to the right.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USceneComponent* _LeftHandPosition;

	/* The time, in seconds, after getting dropped by a NPC, that this weapon will get destroyed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (DisplayName = "Despawn time"))
	float _DespawnTime;


	/////////////////////////////////////////////////////
					/* Misc */
	/////////////////////////////////////////////////////
protected:
	/* The owner's animation instance */
	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	UAnimInstance* _AnimInstance;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	bool _bCanShoot;	

	/* The pawn that currently owns and carries this weapon */
	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	AHumanoid* _MyOwner;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FVector GetForwardCameraVector() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AttachMeshToPawn();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void DetachMeshFromPawn();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetOwningPawn(AHumanoid* NewOwner);

	virtual void BeginPlay() override;

	void SetUpMesh();

	UFUNCTION()
	virtual void HandleOnDestroyed(AActor* DestroyedActor);

public:
	void SetEquipOffset(FName Socket);

	AGun();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void OnEquip(AHumanoid* NewOwner);

	/* Un-equip the gun. 
	 * @param bDropGun Set to false if the weapon should go to a holster (no collision and can't fire),
	 * otherwise it will be dropped.
	 * @param bRequestMulticast If true, then the weapon will be un-equipped on the server and all clients
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Unequip(bool bDropGun = false, bool bRequestMulticast = true);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	EWEaponType GetWeaponType() { return _GunStats.WeaponType; }

	virtual UMeshComponent* GetMesh() const override;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	UCameraComponent* GetZoomCamera() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartRespawnTimer();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopRespawnTimer();

private:
	FTimerHandle _DespawnTH;

	void AddToAudioActorCache();
	void RemoveFromAudioActorCache();


	/////////////////////////////////////////////////////
				/* Interactable interface */
	/////////////////////////////////////////////////////
public:
	virtual void OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent) override;


	/////////////////////////////////////////////////////
						/* Firing */
	/////////////////////////////////////////////////////
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	EFireMode _CurrentFireMode;

	/* How many shots are fired already in the current salvo. Used for the increased spread during firing. */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	int32 _SalvoCount;

	/* How many shots are fired already in the current burst (see: burst mode). */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	int32 _BurstCount;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	EWeaponState _CurrentGunState;

	/* The spawned muzzle flash particle system */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	class UParticleSystemComponent* _SpawnedMuzzleFlashComponent;

private:
	FTimerHandle _WeaponCooldownTH;

	/* Used to set the new fire mode, when the player changes the fire mode. */
	int32 _CurrentFireModePointer;

	/* If non-zero, this is the remaining vertical spread (kickback) to apply after the weapon was fired. */
	float _VerticalSpreadToApply;

	/* If non-zero, this is the remaining horizontal spread (kickback) to apply after the weapon was fired. */
	float _HorizontalSpreadToApply;

	/**
	* Adjusts the aim based on lineTraceRange.
	* Makes a line trace from startLocation forwards to StartLocation * lineTraceRange.
	* The line trace is by channel ECollisionChannel::ECC_Camera.
	* If a viable target is hit, then the return rotator points from the muzzle location towards the hit point.
	* If not viable target was hit, direction.Rotation() is returned.
	* @param startLocation The start location where the line trace starts.
	* @param direction The direction in which the line trace will go.
	* @return RotatorIf a viable target was hit, then the target location, otherwise the StartLocation.
	*/
	FVector AdjustAimRotation(FVector startLocation, FVector direction);

	/** Function is called after the WeaponCooldownTimer is called and activated. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ContinousOnFire();

	/* Checks if the weapon is able to fire in an automatic mode (= holding Fire button results in continuous fire) */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool CanRapidFire() const;
	
	/* Adds horizontal and vertical weapon spread to the variables 'm_HorizontalSpreadToApply' and 'm_VerticalSpreadToApply'. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AddWeaponSpread();

	/* Applies the set weapon spread (see @AddWeaponSpread) to this gun's owners controller. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ApplyWeaponSpread(float DeltaSeconds);

	void HandleMuzzleFlash(bool bSpawnMuzzleFlash);
	void PlayFireAnimation();
	void PlayFireSound();
	void SpawnEjectedShell();

public:
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void FireGun();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void OnStopFire();

	/** Returns the number of rounds the weapon can fire each minute. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetRoundsPerMinute() const;

	/** Returns the muzzle sockets location in world space. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	FTransform GetMuzzleTransform() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool CanShoot() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	EFireMode ToggleFireMode();

	/* Sets the fire mode to the given value. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetFireMode(EFireMode NewFireMode);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	EFireMode GetCurrentFireMode() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetCooldownTime() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	AMagazine* GetMagazine() const;


	/////////////////////////////////////////////////////
					/* Reloading */
	/////////////////////////////////////////////////////
protected:
	/* The currently loaded magazine. */
	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	AMagazine* _LoadedMagazine;

	/* Stops the reloading process by stop playing the reload animation. */
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Weapon")
	void Multicast_StopReloading();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void FinishReloadWeapon();

	/**
	 * [Server] Spawns a new magazine from the class that this weapon can use.
	 * Does NOT attach it to anything.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	AMagazine* SpawnNewMagazine(const FTransform& SpawnTransform);

	/**
	 * [Server] Removes a magazine from the owner's inventory.
	 * @return True if the magazine was successfully removed, e.g. there was a magazine in the inventory.
	 * Otherwise false.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool GetAmmoFromInventory();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void DropMagazine();

	/**
	 * Attaches the given magazine to the gun at the correct location on the gun.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AttachMagazine(AMagazine* Magazine);

	/* Checks if this gun's owner has a suitable magazine in his inventory. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool CheckIfOwnerHasMagazine() const;

public:
	/* Will attach the loaded magazine to the character's hand. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void OnAnimNotify_AttachMagToHand();

	/* Drops the attached magazine. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void OnAnimNotify_DropMagazine();

	/* Checks if the owner has ammo to reload. If not, the reload animation will be stopped. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void OnAnimNotify_CheckForAmmo();

	/* Gets a new magazine from the inventory, spawns and attaches it to the character's hand. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void OnAnimNotify_SpawnNewMag();

	/* Attaches the currently held magazine to the gun. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void OnAnimNotify_AttachMagToGun();

	/* Sets the new weapon state. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void OnAnimNotify_FinishReloading();

	/* Reloads the weapon. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Reload();


	/////////////////////////////////////////////////////
					/* Networking */
	/////////////////////////////////////////////////////
private:
	UPROPERTY(ReplicatedUsing = OnItemGrab)
	AItemBase* _ItemToGrab;

	UPROPERTY(ReplicatedUsing = OnMagAttached)
	AMagazine* _MagToAttach;
	
	UFUNCTION()
	void OnItemGrab();

	UFUNCTION()
	void OnMagAttached();

	UFUNCTION(NetMulticast, Unreliable)
	void HandleOnFire_Multicast();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RepMyOwner(AHumanoid* NewOwner);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Reload();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_OnStopFire();

	UFUNCTION(Server, Reliable, WithValidation)
	void OnFire_Server(EFireMode FireMode, FTransform SpawnTransform);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayReloadAnimation();

	UFUNCTION(NetMulticast, Reliable)
	void HandleMuzzleFlash_Multicast(bool bSpawnMuzzleFlash);

	UFUNCTION(NetMulticast, Reliable)
	void Unequip_Multicast(bool bDropGun);

	UFUNCTION(Server, WithValidation, Reliable)
	void Unequip_Server(bool bDropGun);
};