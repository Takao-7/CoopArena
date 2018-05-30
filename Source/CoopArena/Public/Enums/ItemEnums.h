// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ItemEnums.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None,
	Weapon,
	Ammo,
	Tool
};