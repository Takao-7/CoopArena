// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "MyDamageType.h"
#include "MyPhysicalMaterial.generated.h"


/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = Physics)
class COOPARENA_API UMyPhysicalMaterial : public UPhysicalMaterial
{
	GENERATED_BODY()

private:
	/* Base damage modification. All incoming damage will be modified by this value. */
	UPROPERTY(EditDefaultsOnly, Category = "Damage Resistance", meta = (DisplayName = "Damage modification"))
	float _DamageModification;

public:
	UMyPhysicalMaterial();

	UFUNCTION(BlueprintPure, Category = "Damage Resistance")
	FORCEINLINE float GetDamageMod() { return _DamageModification; };
};