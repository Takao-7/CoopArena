// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Magazine.generated.h"


class AProjectile;


UCLASS()
class COOPARENA_API AMagazine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagazine();
	
	/**
	* How many rounds are left in the magazine.
	* A value of -1 means that the magazine has infinite ammo.
	*/
	int32 RoundsLeft();

	/**
	 * Returns the next projectile from the magazine
	 * @param If true, the capacity will be reduced by one.
	 */
	TSubclassOf<AProjectile> GetProjectileClass(bool reduceCapacity = true);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	/* The maximum amount of bullets the magazine contains. */
	UPROPERTY(EditDefaultsOnly, Category = Magazine)
	int32 m_Capacity;	

	UPROPERTY(EditDefaultsOnly, Category = Magazine)
	TSubclassOf<AProjectile> m_ProjectileType;
};