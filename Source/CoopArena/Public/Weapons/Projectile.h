// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"


class UProjectileMovementComponent;
class UCapsuleComponent;
class UStaticMeshComponent;
class UMyDamageType;
class UArrowComponent;
namespace ForwardDeclare { class USoundBase; }


USTRUCT(BlueprintType)
struct FProjectileValues
{
	GENERATED_BODY()

	FProjectileValues()
	{
		BaseDamage = 20.0f;
		CriticalHitDamageMultiplier = 10.0f;
		bLinearDamageDropOff = true;
		DamageDropOffPerSecond = 1.0f;
		lifeTime = 5.0f;
	}

	/* The projectiles base damage, without any modifiers */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	float BaseDamage;

	/* With how much the damage is multiplied when a critical hit is scored */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	float CriticalHitDamageMultiplier;

	/* The damage drop-off factor per second flight */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	float DamageDropOffPerSecond;

	/* False = exponential damage drop-off */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	bool bLinearDamageDropOff;

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


	/////////////////////////////////////////////////////
					/* Components */
	/////////////////////////////////////////////////////
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	UCapsuleComponent* Collision;

	///////////////////////////////////////////////////
					/* Parameters */
	///////////////////////////////////////////////////
protected:
	/* The sound that will be played when this projectile flies by a player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	USoundBase* _FlyBySound;

	/* The case that is spawned after this projectile is fired. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile, meta = (DisplayName = "Projectile case"))
	TSubclassOf<AActor> _ProjectileCase;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	FProjectileValues _ProjectileValues;

	UPROPERTY(BlueprintReadOnly, Category = Projectile)
	float _TimeSecondsWhenSpawned;

	UPROPERTY(BlueprintReadOnly, Category = Projectile)
	float _RemainingPenetration;

	/* The default hit effect that is played, when no specific hit effect is found or defined for the target material */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	UParticleSystem* _DefaultHitEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	USoundBase* _DefaultHitSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile)
	UParticleSystem* _TrailEffect;
	

	/////////////////////////////////////////////////////
					/* Functions */
	/////////////////////////////////////////////////////
public:
	AProjectile();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Projectile)
	float GetDamageWithFallOff() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Projectile)
	TSubclassOf<AActor> GetProjectileCase() const { return _ProjectileCase; };

	void PlayFlyBySound();
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = Projectile)
	FVector GetImpulse() const;

	UFUNCTION(BlueprintCallable, Category = Projectile)
	void HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


	/////////////////////////////////////////////////////
					/* Networking */
	/////////////////////////////////////////////////////
protected:
	UFUNCTION(NetMulticast, Reliable)
	void ApplyDamage_Multicast(AActor* OtherActor, float Damage, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Unreliable)
	void HandleHitEffects(UParticleSystem* ImpactEffect, FVector Location, FRotator Rotation, USoundBase* ImpactSound, UPrimitiveComponent* OtherComp);
};
