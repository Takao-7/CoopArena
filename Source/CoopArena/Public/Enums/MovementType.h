// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MovementType.generated.h"

UENUM(BlueprintType)
enum class EMovementType : uint8
{
		Idle,
		IdleCrouch,
		Crouching,
		Walking,
		Jogging,
		Sprinting
};
