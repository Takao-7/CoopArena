// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gun.generated.h"


class AHumanoid;
class AProjectile;
//class UUserWidget;


UENUM(BlueprintType)
enum class EFireMode : uint8
{
	 Single,
	 /*Burst,*/
	 Auto
};


UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Idle,
	Firing,
	Reloading,
	Equipping
};


UCLASS()
class COOPARENA_API AGun : public AActor
{
	GENERATED_BODY()
	
protected:
	/** Name of the bone or socket for the muzzle */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FName m_MuzzleAttachPoint;

	/** Sound to play each time we fire */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	class USoundBase* m_FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	class UAnimMontage* m_FireAnimation;

	UPROPERTY(BlueprintReadOnly, BlueprintReadOnly, Category = Weapon)
	class UAnimInstance* m_AnimInstance;

	/**
	 *	The type of magazine that this weapon can be used with.
	 */
	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	TSubclassOf<AMagazine> m_UsableMagazineTyp;*/

	/**
	 *	The currently loaded magazine.
	 */
	/*UPROPERTY(BlueprintReadOnly, Category = Weapon)
	AMagazine* m_LoadedMagazine;*/

	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	bool m_bCanShoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Weapon)
	int32 m_MagazineSize;

	UPROPERTY(BlueprintReadWrite, Category = Weapon)
	int32 m_ShotsLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	TSubclassOf<AProjectile> m_ProjectileToSpawn;

	///* 
	// * How many shots are fired when the weapon fires a burst shot,
	// * IF the weapon has that mode. Otherwise this value will do nothing.
	// */
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	//int32 m_ShotsPerBurst;

	///* How many shots are left in the burst */
	//UPROPERTY(BlueprintReadWrite, Category = Weapon)
	//int32 m_BurstCount;

	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	UUserWidget* m_InfoWidget;*/

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	USkeletalMeshComponent* m_Mesh;

	/* The pawn that currently owns and carries this weapon */
	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	AHumanoid* m_MyOwner;

	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	EWeaponState m_CurrentState;

	/* 
	 * Time, in seconds, between each shot. If this value is <= 0, then the weapon can only fire
	 * in Single mode, no matter what fire modes it has. 
	 */
	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	float m_Cooldown;

	/**
	* The maximum distance the gun will cast a ray when firing to adjust the aim.
	* If there isn't any viable target in that range, the shoot will travel at a strait line
	* from the barrel in the owner's view direction.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	float m_lineTraceRange;

	///** The gun's fire modes */
	//UPROPERTY(EditDefaultsOnly, Category = Weapon)
	//TArray<EFireMode> m_FireModes;

	FTimerHandle m_WeaponCooldownTimer;
	float m_SpreadHorizontal;
	float m_SpreadVertical;
	EFireMode m_CurrentFireMode;

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

	///* PickUp Interface */
	//void OnPickUp(AActor* NewOwner);
	//void OnDrop();
	//UUserWidget* OnBeginTraceCastOver(APawn* TracingPawn);
	//void OnEndTraceCastOver(APawn* TracingPawn);
	///* PickUp Interface end */
	//
	///* Tool Interface */
	//void UseTool();
	//void UseToolSecondary();
	//void ReloadTool();
	//void StopUsingTool();
	float GetCooldownTime() const;
	//float GetCooldownTimeSecondary() const;
	//void Equip(AHumanoid* NewOwner);
	//void Unequip();
	///* Tool Interface end */
};