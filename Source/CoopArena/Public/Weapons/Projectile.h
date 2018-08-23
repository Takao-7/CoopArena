// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"


class UProjectileMovementComponent;
class UCapsuleComponent;
class UStaticMeshComponent;
class UMyDamageType;


USTRUCT(BlueprintType)
struct FProjectileValues
{
	GENERATED_BODY()

	FProjectileValues()
	{
		BaseDamage = 20.0f;
		Mass = 0.01;
		CriticalHitDamageMultiplier = 10.0f;
		bLinealDamageDropOff = true;
		DamageDropOffPerSecond = 1.0f;
		lifeTime = 5.0f;
		Penetration = 10.0f;
		bPenetrationIsVelocityBound = true;
	}

	/* The projectiles base damage, without any modifiers */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	float BaseDamage;

	/* This projectile's penetration against RHA, in mm. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	float Penetration;

	/* Is the penetration value proportional (=bound) to the velocity (e.g. a normal projectile) ? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	bool bPenetrationIsVelocityBound;

	/* Projectile's mass in kg. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	float Mass;

	/* With how much the damage is multiplied when a critical hit is scored */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	float CriticalHitDamageMultiplier;

	/* The damage drop-off factor per second flight */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	float DamageDropOffPerSecond;

	/* False = exponential damage drop-off */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	bool bLinealDamageDropOff;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	TSubclassOf<UMyDamageType> DamageType;

	/* Time in seconds that this projectile will life. 0 means that this projectile will not have a limited lifetime. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile, meta = (ClampMin = 0.0f))
	float lifeTime;
};

UCLASS()
class COOPARENA_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Projectile)
	float GetDamageWithFallOff() const;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = Projectile)
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	/* Function to check if the bullet will hit something in the next frame. Called at begin play and on every frame. */
	UFUNCTION(BlueprintCallable, Category = Projectile)
	void HitDetectionLineTrace();

	FVector GetImpulse();

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = Projectile)
	UProjectileMovementComponent* _ProjectileMovementComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	FProjectileValues _ProjectileValues;

	UPROPERTY(BlueprintReadOnly, Category = Projectile)
	float _TimeSecondsWhenSpawned;

	UPROPERTY(BlueprintReadOnly, Category = Projectile)
	float _RemainingPenetration;

	/* The default hit effect that is played, when no specific hit effect is found or defined for the target material */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	UParticleSystem* _DefaultHitEffect;

public:
	/* The controller who shot this projectile */
	UPROPERTY(BlueprintReadWrite, Category = Projectile)
	AController* _Instigator;
};
