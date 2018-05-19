// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"


class UProjectileMovementComponent;
class UCapsuleComponent;
class UStaticMeshComponent;


USTRUCT(BlueprintType)
struct FProjectileValues
{
	GENERATED_BODY()

	FProjectileValues()
	{
		BaseDamage = 20.0f;
		CriticalHitDamageMultiplier = 10.0f;
		Speed = 1000.0f;
		bLinealDamageDropOff = true;
		DamageDropOffPerSecond = 0.5f;
	}

	/* The projectiles base damage, without any modifiers */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	float BaseDamage;

	/* With how much the damage is multiplied when a critical hit is scored */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	float CriticalHitDamageMultiplier;

	/* Projectile speed in m/s */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	float Speed;

	/* The damage drop-off factor per second flight */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	float DamageDropOffPerSecond;

	/* False = exponential damage drop-off */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	bool bLinealDamageDropOff;
};

UCLASS()
class COOPARENA_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

	UFUNCTION(BlueprintPure, Category = Projectile)
	float GetDamageWithFallOff() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	UProjectileMovementComponent* _ProjectileMovementComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	UStaticMeshComponent* _Mesh;

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	UCapsuleComponent* _CollisionCapsule;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	FProjectileValues _ProjectileValues;

	UPROPERTY(BlueprintReadOnly, Category = Projectile)
	float _TimeWhenSpawned;

	/* The default hit effect that is played, when no specific hit effect is found or defined for the target material */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	UParticleSystem* _DefaultHitEffect;

public:
	/* The controller how shot this projectile */
	UPROPERTY(BlueprintReadWrite, Category = Projectile)
	AController* _Instigator;
};
