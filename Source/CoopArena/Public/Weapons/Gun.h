// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "Weapons/Weapon.h"
#include "Gun.generated.h"


UENUM(BlueprintType)
enum class EFireMode : uint8
{
	 Single,
	 Burst,
	 Auto
};


UCLASS()
class COOPARENA_API AGun : public AWeapon
{
	GENERATED_BODY()
	
protected:
	/** Name of the bone or socket for the muzzle */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FName MuzzleAttachPoint;

	/** Sound to play each time we fire */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	class UAnimMontage* FireAnimation;

	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	class UAnimInstance* AnimInstance;

	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	bool bCanShoot;

	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	int32 SalvoCount;

	/**
	* The maximum distance the gun will cast a ray when firing to adjust the aim.
	* If there isn't any viable target in that range, the shoot will travel at a strait line
	* from the barrel in the owner's view direction.
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	float lineTraceRange;

	/** The gun's fire modes */
	UPROPERTY(EditDefaultsOnly, EditFixedSize, Category = Weapon)
	TArray<EFireMode> FireModes;

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
	FRotator AdjustAimRotation(FVector startLocation, FVector direction);	

	/** Internal function that is called when the weapon fires */
	void OnFire();

	/** Function is called after the WeaponCooldownTimer is called and activated. */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void ContinousOnFire();

public:
	AGun();

	/** Returns the number of rounds the weapon can fire each minute. */
	UFUNCTION(BlueprintPure, Category = Weapon)
	float GetRoundsPerMinute() const;
	
	/** Returns the muzzle sockets location in world space. */
	UFUNCTION(BlueprintPure, Category = Weapon)
	FVector GetMuzzleLocation() const;

	UFUNCTION()
	virtual bool CanShoot() const override;
};