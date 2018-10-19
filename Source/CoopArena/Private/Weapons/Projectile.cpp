// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "CoopArena.h"
#include "MyPhysicalMaterial.h"
#include "UnrealNetwork.h"


// Sets default values
AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetGenerateOverlapEvents(true);
	Mesh->SetCollisionResponseToAllChannels(ECR_Overlap);
	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Projectile, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_ProjectilePenetration, ECR_Ignore);
	Mesh->bReturnMaterialOnMove = true;
	Mesh->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::HandleOverlap);
	RootComponent = Mesh;

	_ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));

	SetReplicates(true);
	SetReplicateMovement(true);
}

//////////////////////////////////////////////////////////////////////////////////////
float AProjectile::GetDamageWithFallOff() const
{
	float flightTime = 0.0f;
	if (_TimeSecondsWhenSpawned != 0.0f)
	{
		flightTime = GetWorld()->GetTimeSeconds() - _TimeSecondsWhenSpawned;
	}
	float damage;
	if (_ProjectileValues.bLinealDamageDropOff)
	{
		damage = _ProjectileValues.BaseDamage - (flightTime * _ProjectileValues.DamageDropOffPerSecond);
	}
	else
	{
		damage = _ProjectileValues.BaseDamage - (flightTime * FMath::Pow(_ProjectileValues.DamageDropOffPerSecond, 2));
	}
	return damage;
}

//////////////////////////////////////////////////////////////////////////////////////
FVector AProjectile::GetImpulse() const
{
	return GetActorForwardVector() * (GetVelocity().Size() * _ProjectileValues.Mass);
}

//////////////////////////////////////////////////////////////////////////////////////
void AProjectile::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || OtherActor == nullptr)
	{
		return;
	}

	UMyPhysicalMaterial* material = Cast<UMyPhysicalMaterial>(SweepResult.PhysMaterial.Get());	
	float damage = GetDamageWithFallOff();
	if (material)
	{
		damage *= material->GetDamageMod();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No MyPhysicalMaterial on: %s"), *OtherActor->GetName());
	}

	if (OtherActor->GetInstigator())
	{
		UGameplayStatics::ApplyPointDamage(OtherActor, damage, GetImpulse(), SweepResult, GetOwner()->GetInstigatorController(), this, _ProjectileValues.DamageType);
	}
	else  // We hit the environment.
	{
		ApplyDamage_Multicast(OtherActor, damage, SweepResult);
	}

	UParticleSystem* hitEffect = nullptr;
	if (_ProjectileValues.DamageType)
	{
		UMyDamageType* damageObj = Cast<UMyDamageType>(_ProjectileValues.DamageType->GetDefaultObject(true));
		hitEffect = damageObj->GetHitEffect(UPhysicalMaterial::DetermineSurfaceType(material));
	}

	SpawnHitEffect_Multicast(hitEffect ? hitEffect : _DefaultHitEffect, SweepResult.ImpactPoint, SweepResult.ImpactNormal.Rotation());
	Destroy();
}

//////////////////////////////////////////////////////////////////////////////////////
void AProjectile::BeginPlay()
{
	Super::BeginPlay();	

	_TimeSecondsWhenSpawned = GetWorld()->GetTimeSeconds();
	SetLifeSpan(_ProjectileValues.lifeTime);
}

//////////////////////////////////////////////////////////////////////////////////////
void AProjectile::ApplyDamage_Multicast_Implementation(AActor* OtherActor, float Damage, const FHitResult& SweepResult)
{
	UGameplayStatics::ApplyPointDamage(OtherActor, Damage, GetImpulse(), SweepResult, GetOwner()->GetInstigatorController(), this, _ProjectileValues.DamageType);
}

//////////////////////////////////////////////////////////////////////////////////////
void AProjectile::SpawnHitEffect_Multicast_Implementation(UParticleSystem* Effect, FVector Location, FRotator Rotation)
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Effect, Location, Rotation, true, EPSCPoolMethod::None);
}