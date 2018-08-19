// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Enums/WeaponEnums.h"
#include "Enums/BAS_Enums.h"
#include "BASVariables.generated.h"

USTRUCT(BlueprintType)
struct FBASVariables
{
	GENERATED_BODY()

	/* The yaw angle between the control rotation and the input direction. */
	UPROPERTY(BlueprintReadOnly)
	float InputDirection;

	/* The pitch angle between the control rotation and the actor's pitch.  */
	UPROPERTY(BlueprintReadOnly)
	float AimPitch;

	/* The actor's horizontal speed. */
	UPROPERTY(BlueprintReadOnly)
	float HorizontalVelocity;

	/* Is the actor moving forward. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsMovingForward;

	/* Is the actor aiming through his weapon's ironsights. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsAiming;

	/* The actor's current movement type. */
	UPROPERTY(BlueprintReadOnly)
	EMovementType MovementType;

	UPROPERTY(BlueprintReadOnly)
	EMovementAdditive MovementAdditive;

	UPROPERTY(BlueprintReadOnly)
	EWEaponType EquippedWeaponType;
};
