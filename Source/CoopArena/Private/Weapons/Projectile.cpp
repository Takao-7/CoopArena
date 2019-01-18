// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "Engine/World.h"
#include "CoopArena.h"
#include "MyPhysicalMaterial.h"
#include "UnrealNetwork.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "TimerManager.h"


AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);
	SetReplicateMovement(true);

	Collision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetGenerateOverlapEvents(true);
	Collision->SetNotifyRigidBodyCollision(true);
	Collision->SetCollisionResponseToAllChannels(ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Projectile, ECR_Ignore);
	Collision->SetCollisionObjectType(ECC_Projectile);
	Collision->SetUseCCD(true);
	Collision->bReturnMaterialOnMove = true;
	Collision->OnComponentHit.AddDynamic(this, &AProjectile::HandleHit);
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::HandleBeginOverlap);
	Collision->SetCapsuleSize(0.3f, 1.0f);
	RootComponent = Collision;
	Collision->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->SetNotifyRigidBodyCollision(false);
	Mesh->SetCollisionObjectType(ECC_Projectile);
	Mesh->SetupAttachment(RootComponent);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
}

//////////////////////////////////////////////////////////////////////////////////////
float AProjectile::GetDamageWithFallOff() const
{
	float flightTime = _TimeSecondsWhenSpawned != 0.0f ? GetWorld()->GetTimeSeconds() - _TimeSecondsWhenSpawned : 0;

	float damage;
	if (_ProjectileValues.bLinearDamageDropOff)
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
void AProjectile::PlayFlyBySound()
{
	UGameplayStatics::SpawnSoundAttached(_FlyBySound, Mesh);
}

//////////////////////////////////////////////////////////////////////////////////////
FVector AProjectile::GetImpulse() const
{
	return GetActorForwardVector() * (GetVelocity().Size());
}

//////////////////////////////////////////////////////////////////////////////////////
void AProjectile::HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (HasAuthority() == false)
	{
		return;
	}

	UParticleSystem* hitEffect = _DefaultHitEffect;
	USoundBase* impactSound = _DefaultHitSound;

	UMyPhysicalMaterial* material = Cast<UMyPhysicalMaterial>(Hit.PhysMaterial.Get());
	float damage = GetDamageWithFallOff();
	if (material)
	{
		damage *= material->GetDamageMod();
		hitEffect = material->GetImpactEffect();
		impactSound = material->GetImpactSound();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No MyPhysicalMaterial on: %s"), OtherActor ? *OtherActor->GetName() : TEXT("Target"));
	}

	if (OtherActor && OtherActor->GetInstigator())
	{
		UGameplayStatics::ApplyPointDamage(OtherActor, damage, GetImpulse(), Hit, GetOwner()->GetInstigatorController(), this, _ProjectileValues.DamageType);
	}
	else  // We hit the environment.
	{
		ApplyDamage_Multicast(OtherActor, damage, Hit);
	}

	if (hitEffect || impactSound)
	{
		HandleHitEffects(hitEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), impactSound, OtherComp);
	}

	Destroy();
}

//////////////////////////////////////////////////////////////////////////////////////
void AProjectile::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UMyPhysicalMaterial* material = Cast<UMyPhysicalMaterial>(SweepResult.PhysMaterial.Get());
	const bool bIsGlass = material && material->IsDestructableGlass();

	if (!HasAuthority() || !bIsGlass)
	{
		return;
	}

	UParticleSystem* hitEffect = material->GetImpactEffect();
	if (hitEffect == nullptr)
	{
		hitEffect = _DefaultHitEffect;
	}
	USoundBase* impactSound = material->GetImpactSound();
	if (impactSound == nullptr)
	{
		impactSound = _DefaultHitSound;
	}

	if (hitEffect || impactSound)
	{
		HandleHitEffects(hitEffect, SweepResult.ImpactPoint, SweepResult.ImpactNormal.Rotation(), impactSound, OtherComp);
	}
	
	ApplyDamage_Multicast(OtherActor, GetDamageWithFallOff(), SweepResult);
}

//////////////////////////////////////////////////////////////////////////////////////
void AProjectile::HandleOnDestroyed(AActor* DestroyedActor)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(DestroyedActor);
}

//////////////////////////////////////////////////////////////////////////////////////
void AProjectile::HandleHitEffects_Implementation(UParticleSystem* ImpactEffect, FVector Location, FRotator Rotation, USoundBase* ImpactSound, UPrimitiveComponent* OtherComp)
{
	if (ImpactSound)
	{
		UGameplayStatics::SpawnSoundAttached(ImpactSound, OtherComp);
	}

	if (ImpactEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Location, Rotation, true, EPSCPoolMethod::None);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	UParticleSystemComponent* trailEffect = UGameplayStatics::SpawnEmitterAttached(_TrailEffect, RootComponent, NAME_None, FVector::ZeroVector, FRotator(90.0f, 0.0f, 0.0f));
	if (trailEffect == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("No trail effect spawned."));
	}

	if(HasAuthority())
	{
		OnDestroyed.AddDynamic(this, &AProjectile::HandleOnDestroyed);
		_TimeSecondsWhenSpawned = GetWorld()->GetTimeSeconds();
		SetLifeSpan(_ProjectileValues.lifeTime);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void AProjectile::ApplyDamage_Multicast_Implementation(AActor* OtherActor, float Damage, const FHitResult& SweepResult)
{
	UGameplayStatics::ApplyPointDamage(OtherActor, Damage, GetImpulse(), SweepResult, GetOwner()->GetInstigatorController(), this, _ProjectileValues.DamageType);
}