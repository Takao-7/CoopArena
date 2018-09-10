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
	/* Rolled homogeneous amour equivalent against kinetic damage. Comparison factor for how well this material can protect from kinetic damage in comparison to steel.
	 * A value of 1 would mean that this material is equivalent to steel. A value of 0.5 means, that this material is only half as good as steel
	 * (e.g. you would need 2 mm of this material to achieve the same protection level against kinetic damage than 1 mm of steel).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Resistance", meta = (DisplayName = "RHA Kinetic"))
	float m_RHA_Kinetic;

	/* Rolled homogeneous amour equivalent against explosive damage. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Resistance", meta = (DisplayName = "RHA Explosive"))
	float m_RHA_Explosive;

	/* The physical thickness of this amour. Use ONLY if the amour and the body are NOT separate components.
	 * Set to 0 if this is material is on a mesh that is NOT the actual body of a person (e.g. the scenery, etc.).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Resistance", meta = (DisplayName = "Thickness"))
	float m_Thickness;

	/* Base damage modification. All incoming damage will be modified by this value. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Resistance", meta	= (DisplayName = "Damage modification"))
	float m_DamageModification;

public:
	UMyPhysicalMaterial();

	UFUNCTION(BlueprintPure, Category = "Damage Resistance")
	FORCEINLINE float GetRHA_Kinetic() { return m_RHA_Kinetic; };

	UFUNCTION(BlueprintPure, Category = "Damage Resistance")
	FORCEINLINE float GetRHA_Explosive() { return m_RHA_Explosive; };

	UFUNCTION(BlueprintPure, Category = "Damage Resistance")
	FORCEINLINE float GetDamageMod() { return m_DamageModification; };

	/* 
	 * Calculates the effective thickness of this armor (or object).
	 * @param ImpactPoint The impact point from where to calculate the effective thickness.
	 * @param Direction The direction where the line trace will go.
	 * @param RayLength The ray trace length. This should be at least the max penetration value that the projectile has against this material.
	 * @return The Effective amour thickness.
	 */
	UFUNCTION(BlueprintPure, Category = "Damage Resistance")
	float GetAmourThickness(FVector ImpactPoint, FVector Direction, float RayLength = 500.0f);
};