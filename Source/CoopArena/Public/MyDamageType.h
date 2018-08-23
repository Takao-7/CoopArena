// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "MyDamageType.generated.h"


class UMyPhysicalMaterial;
class UParticleSystem;


/**
 * 
 */
UCLASS()
class COOPARENA_API UMyDamageType : public UDamageType
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TMap<TEnumAsByte<EPhysicalSurface>, UParticleSystem*> _HitEffects;

public:
	/* Returns the damage modifier from this damage type against the given material. */
	UFUNCTION(BlueprintPure, Category = Damage)
	float GetDamageModAgainstMaterial(UMyPhysicalMaterial* Material);

	UFUNCTION(BlueprintPure, Category = Effects)
	FORCEINLINE UParticleSystem* GetHitEffect(EPhysicalSurface Surface) const;
};
