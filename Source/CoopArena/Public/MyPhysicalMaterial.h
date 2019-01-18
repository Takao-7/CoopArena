// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "MyDamageType.h"
#include "MyPhysicalMaterial.generated.h"


UCLASS(BlueprintType, Blueprintable, ClassGroup = Physics)
class COOPARENA_API UMyPhysicalMaterial : public UPhysicalMaterial
{
	GENERATED_BODY()

private:
	/* Base damage modification. All incoming damage will be modified by this value. */
	UPROPERTY(EditDefaultsOnly, Category = "Damage Resistance", meta = (DisplayName = "Damage modification"))
	float _DamageModification;

	UPROPERTY(EditDefaultsOnly, Category = "Effects", meta = (DisplayName = "Projectile impact effect"))
	UParticleSystem* _ProjectileImpactEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effects", meta = (DisplayName = "Projectile impact sound"))
	USoundBase* _ProjectileImpactSound;

	UPROPERTY(EditDefaultsOnly, Category = "Effects", meta = (DisplayName = "Footstep sound"))
	USoundBase* _FootstepSound;

	UPROPERTY(EditDefaultsOnly, Category = "Material", meta = (DisplayName = "Is destructable glass"))
	bool _bIsDestructableGlass;

public:
	UMyPhysicalMaterial();

	UFUNCTION(BlueprintPure, Category = "Damage Resistance")
	FORCEINLINE float GetDamageMod() const { return _DamageModification; };

	UFUNCTION(BlueprintPure, Category = "Effects")
	FORCEINLINE UParticleSystem* GetImpactEffect() { return _ProjectileImpactEffect; };

	UFUNCTION(BlueprintPure, Category = "Effects")
	FORCEINLINE USoundBase* GetImpactSound() { return _ProjectileImpactSound; };

	UFUNCTION(BlueprintPure, Category = "Effects")
	FORCEINLINE USoundBase* GetFootStepSound() { return _FootstepSound; };

	UFUNCTION(BlueprintPure, Category = "Material")
	FORCEINLINE bool IsDestructableGlass() const { return _bIsDestructableGlass; };
};