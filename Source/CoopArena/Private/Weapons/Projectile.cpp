// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "CoopArena.h"
#include "PhysicalMaterials/PhysicalMaterial.h"


// Sets default values
AProjectile::AProjectile()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetGenerateOverlapEvents(true);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	Mesh->bReturnMaterialOnMove = true;
	Mesh->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	RootComponent = Mesh;

	_ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
}


float AProjectile::GetDamageWithFallOff() const
{
	float flightTime = GetWorld()->GetTimeSeconds() - _TimeWhenSpawned;
	float damage;
	if (_ProjectileValues.bLinealDamageDropOff)
	{
		damage = _ProjectileValues.BaseDamage - (flightTime * _ProjectileValues.DamageDropOffPerSecond);
	}
	else
	{
		damage = _ProjectileValues.BaseDamage - (FMath::Pow(flightTime, 2) * _ProjectileValues.DamageDropOffPerSecond);
	}
	return damage;
}


float AProjectile::GetDamageMultiplicatorAgainstSurfaceType(UPhysicalMaterial* Material) const
{
	switch (UPhysicalMaterial::DetermineSurfaceType(Material))
	{
	case SurfaceType_Body:
		return 1.0f;
		break;
	case SurfaceType_Critical:
		return _ProjectileValues.CriticalHitDamageMultiplier;
		break;
	}
	return 1.0f;
}


void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Projectile hit %s"), *OtherActor->GetName());
		FDamageEvent damageEvent;
		damageEvent.DamageTypeClass = _ProjectileValues.DamageType;
		float damage = GetDamageWithFallOff();

		if (Hit.PhysMaterial != nullptr)
		{
			damage *= GetDamageMultiplicatorAgainstSurfaceType(&(*Hit.PhysMaterial));
		}
		UGameplayStatics::ApplyPointDamage(OtherActor, damage, NormalImpulse, Hit, _Instigator, OtherActor, _ProjectileValues.DamageType);

		/*if (Hit.PhysMaterial != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Physical material: %s"), *Hit.PhysMaterial->GetName());
		}*/
	}
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), _DefaultHitEffect, Hit.Location, Hit.Normal.Rotation(), true, EPSCPoolMethod::None);
	Destroy();
}


// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();	

	_TimeWhenSpawned = GetWorld()->GetTimeSeconds();

	SetLifeSpan(10.0f);
}