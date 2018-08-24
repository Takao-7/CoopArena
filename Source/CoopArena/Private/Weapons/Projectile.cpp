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
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetGenerateOverlapEvents(true);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	Mesh->bReturnMaterialOnMove = true;
	//Mesh->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
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
		damage = _ProjectileValues.BaseDamage - (FMath::Pow(flightTime, 2) * _ProjectileValues.DamageDropOffPerSecond);
	}
	return damage;
}


void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	UParticleSystem* particleSystem = _DefaultHitEffect;
	if (OtherActor)
	{
		float damage = GetDamageWithFallOff();
		UMyPhysicalMaterial* material = Cast<UMyPhysicalMaterial>(Hit.PhysMaterial.Get());

		if (Hit.PhysMaterial == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("No PhysMaterial on: %s"), *OtherActor->GetName());
		}

		if (material != nullptr)
		{
			damage *= material->GetDamageMod();

			UMyDamageType* damageObj = Cast<UMyDamageType>(_ProjectileValues.DamageType->GetDefaultObject());
			particleSystem = damageObj->GetHitEffect(UPhysicalMaterial::DetermineSurfaceType(material));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No MyPhsicalMaterial on: %s"), *OtherActor->GetName());
		}
		UGameplayStatics::ApplyPointDamage(OtherActor, damage, NormalImpulse, Hit, GetOwner()->GetInstigatorController(), this, _ProjectileValues.DamageType);
	}
	else
	{
		FString compName = "";
		GetOwner() ? compName = GetOwner()->GetName() : compName = "(no hit component)";
		UE_LOG(LogTemp, Warning, TEXT("%s fired by %s: No hit actor! Hit component: %s"), *GetName(), *GetOwner()->GetName(), *compName);
	}

	if (particleSystem)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), particleSystem, Hit.Location, Hit.Normal.Rotation(), true, EPSCPoolMethod::None);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No particle system found for %s, fired by %s. Hit actor: %s"), *GetName(), *GetInstigator()->GetName(), *OtherActor->GetName());
	}
	Destroy();
}


void AProjectile::HitDetectionLineTrace(float DeltaTime)
{
	FVector traceStart = GetActorLocation();
	FVector traceEnd = traceStart + (GetActorForwardVector() * GetVelocity().Size() * DeltaTime);

	FCollisionQueryParams params;
	params.AddIgnoredActor(GetOwner());
	params.bReturnPhysicalMaterial = true;
	FHitResult hitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(hitResult, traceStart, traceEnd, ECC_ProjectilePenetration, params);

	if (bHit)
	{
		FVector impulse = GetImpulse();
		OnHit(Mesh, hitResult.GetActor(), hitResult.GetComponent(), impulse, hitResult);
	}
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
	UParticleSystem* hitEffect = nullptr;
	UMyPhysicalMaterial* material = Cast<UMyPhysicalMaterial>(SweepResult.PhysMaterial.Get());

	if (material)
	{
		damage *= material->GetDamageMod();

		UMyDamageType* damageObj = Cast<UMyDamageType>(_ProjectileValues.DamageType->GetDefaultObject());
		hitEffect = damageObj->GetHitEffect(UPhysicalMaterial::DetermineSurfaceType(material));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No MyPhysicalMaterial on: %s"), *OtherActor->GetName());
	}

	if (hitEffect == nullptr)
	{
		hitEffect = _DefaultHitEffect;
		if (hitEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), hitEffect, SweepResult.Location, SweepResult.Normal.Rotation(), true, EPSCPoolMethod::None);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No particle system found for %s, fired by %s. Hit actor: %s"), *GetName(), *GetInstigator()->GetName(), *OtherActor->GetName());
		}
	}

	UGameplayStatics::ApplyPointDamage(OtherActor, damage, GetImpulse(), SweepResult, GetOwner()->GetInstigatorController(), this, _ProjectileValues.DamageType);

	Destroy();
}


void AProjectile::BeginPlay()
{
	Super::BeginPlay();	

	_TimeSecondsWhenSpawned = GetWorld()->GetTimeSeconds();
	SetLifeSpan(_ProjectileValues.lifeTime);
	//HitDetectionLineTrace(1/60.0f);
}


void AProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	//HitDetectionLineTrace(DeltaSeconds);
}
