// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"


class UProjectileMovementComponent;


UCLASS()
class COOPARENA_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleDefaultsOnly)
	UProjectileMovementComponent* ProjectileMovementComponent;

public:
	/* The rotation offset that will be applied when spawning this projectile. */
	UPROPERTY(EditAnywhere, Category = Projectile)
	FRotator SpawnRotationOffset;
	
};
