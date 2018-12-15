// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BAS_Enums.generated.h"

UENUM(BlueprintType)
enum class EMovementType : uint8
{
		Idle,
		Moving,
		Sprinting,
		InAir
};


UENUM(BlueprintType)
enum class EMovementAdditive : uint8
{
	None,
	Crouch,
	Prone
};


UENUM(BlueprintType)
enum class EGait : uint8
{
	Walking,
	Jogging,
	Sprinting
};