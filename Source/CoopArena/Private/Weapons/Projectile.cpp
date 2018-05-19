// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"


// Sets default values
AProjectile::AProjectile()
{
	_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	_Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	_Mesh->SetupAttachment(_CollisionCapsule);
	RootComponent = _Mesh;

	/*_CollisionCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision Sphere"));
	_CollisionCapsule->SetCollisionResponseToAllChannels(ECR_Block);
	_CollisionCapsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	_CollisionCapsule->SetCollisionObjectType(ECC_PhysicsBody);
	_CollisionCapsule->SetSimulatePhysics(false);
	_CollisionCapsule->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	_CollisionCapsule->SetupAttachment(RootComponent);*/

	_ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	_ProjectileMovementComponent->bForceSubStepping = true;	
	_ProjectileMovementComponent->InitialSpeed = _ProjectileValues.Speed;
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
}