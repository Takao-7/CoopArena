// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "Magazine.generated.h"


class AProjectile;


UCLASS()
class COOPARENA_API AMagazine : public AItemBase
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

	int32 GetCapacity();

	/**
	 * Removes one or more rounds from the magazine (default = 1).
	 * @return True if the capacity was high enough to remove numRounds.
	 */
	bool RemoveRound(int32 numRounds = 1);

	/**
	 * Returns the magazines projectile class.
	 */
	TSubclassOf<AProjectile> GetProjectileClass();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	/* The maximum amount of bullets the magazine contains. */
	UPROPERTY(EditDefaultsOnly, Category = Magazine)
	int32 _Capacity;

	UPROPERTY(BlueprintReadWrite, Category = Magazine)
	int32 _RoundsLeft;

	UPROPERTY(EditDefaultsOnly, Category = Magazine)
	TSubclassOf<AProjectile> _ProjectileType;
};