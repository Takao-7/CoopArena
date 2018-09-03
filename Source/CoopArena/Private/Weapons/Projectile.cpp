// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "CoopArena.h"
#include "MyPhysicalMaterial.h"


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
}


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


FHitResult AProjectile::HitDetectionLineTrace(float DeltaTime)
{
	FVector traceStart = GetActorLocation();
	FVector traceEnd = traceStart + (GetActorForwardVector() * GetVelocity().Size() * DeltaTime);

	FCollisionQueryParams params;
	params.AddIgnoredActor(GetOwner());
	params.bReturnPhysicalMaterial = true;
	FHitResult hitResult;
	GetWorld()->LineTraceSingleByChannel(hitResult, traceStart, traceEnd, ECC_ProjectilePenetration, params);

	return hitResult;
}


FVector AProjectile::GetImpulse() const
{
	return GetActorForwardVector() * (GetVelocity().Size() * _ProjectileValues.Mass);
}


void AProjectile::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s fired by %s overlapped but did not hit an actor!"), *GetName(), *GetOwner()->GetName());
		return;
	}
	if (OtherComp == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s fired by %s overlapped with an actor but component was hit!"), *GetName(), *GetOwner()->GetName());
	}

	float damage = GetDamageWithFallOff();	
	UMyPhysicalMaterial* material = Cast<UMyPhysicalMaterial>(SweepResult.PhysMaterial.Get());
	if (material)
	{
		damage *= material->GetDamageMod();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No MyPhysicalMaterial on: %s"), *OtherActor->GetName());
	}

	UParticleSystem* hitEffect = nullptr;
	if (_ProjectileValues.DamageType)
	{
		UMyDamageType* damageObj = Cast<UMyDamageType>(_ProjectileValues.DamageType->GetDefaultObject(true));
		hitEffect = damageObj->GetHitEffect(UPhysicalMaterial::DetermineSurfaceType(material));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DamageType on %s is not set!"), *OtherActor->GetName());
	}

	if (hitEffect == nullptr && _DefaultHitEffect)
	{
		hitEffect = _DefaultHitEffect;
	}
	if (hitEffect)
	{	
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), hitEffect, SweepResult.Location, SweepResult.Normal.Rotation(), true, EPSCPoolMethod::None);		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No particle system found for %s, fired by %s. Hit actor: %s"), *GetName(), *GetInstigator()->GetName(), *OtherActor->GetName());
	}

	UGameplayStatics::ApplyPointDamage(OtherActor, damage, GetImpulse(), SweepResult, GetOwner()->GetInstigatorController(), this, _ProjectileValues.DamageType);

	Destroy();
}


void AProjectile::BeginPlay()
{
	Super::BeginPlay();	

	_TimeSecondsWhenSpawned = GetWorld()->GetTimeSeconds();
	SetLifeSpan(_ProjectileValues.lifeTime);
}


void AProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (_TrailEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(_TrailEffect, Mesh, "", FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget);
	}
}
