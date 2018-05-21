// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"


// Sets default values
AProjectile::AProjectile()
{
	_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	_Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	_Mesh->SetupAttachment(_CollisionCapsule);
	RootComponent = _Mesh;

	_ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	_ProjectileMovementComponent->bForceSubStepping = true;
}


float AProjectile::GetDamageWithFallOff() const
{
	float flightTime = GetWorld()->GetTimeSeconds() - _TimeWhenSpawned;
	float damage;
	if (_ProjectileValues.bLinealDamageDropOff)
	{
		damage = _ProjectileValues.BaseDamage * flightTime * _ProjectileValues.DamageDropOffPerSecond;
	}
	else
	{
		damage = _ProjectileValues.BaseDamage * FMath::Pow(flightTime, 2) * _ProjectileValues.DamageDropOffPerSecond;
	}	
	return damage;
}


// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();	

	_TimeWhenSpawned = GetWorld()->GetTimeSeconds();

	SetLifeSpan(10.0f);
}