// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#define ECC_Projectile	ECC_GameTraceChannel1
#define ECC_Item		ECC_GameTraceChannel2


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