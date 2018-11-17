// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "Magazine.generated.h"


class AProjectile;
class UMeshComponent;


UCLASS()
class COOPARENA_API AMagazine : public AItemBase
{
	GENERATED_BODY()
	
public:	
	AMagazine();
	/**
	* How many rounds are left in the magazine.
	* A value of -1 means that the magazine has infinite ammo.
	*/
	UFUNCTION(BlueprintPure, Category = Magazine)
	int32 RoundsLeft() const;

	/* Returns the number of rounds this magazine can hold at maximum. */
	UFUNCTION(BlueprintPure, Category = Magazine)
	int32 GetCapacity() const;

	/**
	 * Removes one or more rounds from the magazine (default = 1).
	 * @return True if the capacity was high enough to remove numRounds.
	 */
	UFUNCTION(BlueprintCallable, Category = Magazine)
	bool RemoveRound(int32 numRounds = 1);

	/**
	 * Returns the magazines projectile class.
	 */
	UFUNCTION(BlueprintPure, Category = Magazine)
	TSubclassOf<AProjectile> GetProjectileClass() const;

	virtual UMeshComponent* GetMesh() const override;

	/* Is this magazine full? */
	bool IsFull() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	/* The maximum amount of bullets the magazine contains. Set to -1 for infinite rounds. */
	UPROPERTY(EditDefaultsOnly, Category = Magazine)
	int32 _Capacity;

	/* How many rounds are left in the magazine. -1 means infinite rounds left. */
	UPROPERTY(Replicated, BlueprintReadWrite, Category = Magazine)
	int32 _RoundsLeft;

	UPROPERTY(EditDefaultsOnly, Category = Magazine)
	TSubclassOf<AProjectile> _ProjectileType;

	UPROPERTY(VisibleDefaultsOnly, Category = Magazine)
	UStaticMeshComponent* _Mesh;
};