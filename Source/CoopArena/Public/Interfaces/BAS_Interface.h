// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Interface.h"
#include "Enums/WeaponEnums.h"
#include "BAS_Interface.generated.h"


UINTERFACE(Blueprintable)
class COOPARENA_API UBAS_Interface : public UInterface
{
	GENERATED_BODY()
};

class COOPARENA_API IBAS_Interface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "BAS Interface")
	bool IsCrouching();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "BAS Interface")
	bool IsAiming();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "BAS Interface")
	EWEaponType GetEquippedWeaponType();
};
