// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponEnums.generated.h"


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
	NoMagazine
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