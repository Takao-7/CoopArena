// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/Gun.h"
#include "Humanoid.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "Weapons/Projectile.h"
#include "Animation/AnimInstance.h"
#include "PlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"


AGun::AGun()
{
	m_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	m_Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	m_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	m_Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	m_Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	m_Mesh->CastShadow = true;
	SetRootComponent(m_Mesh);

	m_CurrentState = EWeaponState::Idle;
}


FVector AGun::AdjustAimRotation(FVector traceStart, FVector direction)
{
	FVector traceEnd = traceStart + direction * m_lineTraceRange;

	// Start the trace further away, in the given direction, so that we don't hit ourself.
	FVector adjustedTraceStart = traceStart + (direction * m_MyOwner->GetCapsuleComponent()->GetScaledCapsuleRadius() * 1.3f);

	FHitResult Hit;
	GetWorld()->LineTraceSingleByChannel(Hit, adjustedTraceStart, traceEnd, ECollisionChannel::ECC_Camera);

	if (!Hit.IsValidBlockingHit())
	{
		return direction;
	}

	FVector adjustetDirection = Hit.Location - GetMuzzleLocation();
	adjustetDirection.Normalize();

	return adjustetDirection;
}


void AGun::ContinousOnFire()
{
	/*if (m_CurrentFireMode == EFireMode::Burst)
	{
		if (m_BurstCount > 0)
		{
			m_BurstCount--;
			UseTool();
		}
		else
		{
			if (GetCooldownTime() == 0)
			{
				m_CurrentFireMode = EFireMode::Single;
			}
			else
			{
				m_CurrentFireMode = EFireMode::Auto;
			}
			m_CurrentState = EWeaponState::Idle;
}
	}
	else if (m_CurrentState == EWeaponState::Firing)
	{
		UseTool();
	}

	if (m_BurstCount < 1)
	{
		m_BurstCount = m_ShotsPerBurst;
	}*/
}


FVector AGun::GetForwardCameraVector() const
{
	if (m_MyOwner->IsPlayerControlled())
	{
		FVector CamPos;
		FRotator CamRot;
		Cast<APlayerController>(m_MyOwner->Controller)->GetPlayerViewPoint(CamPos, CamRot);
		return CamRot.Vector();
	}
	else
	{
		return m_MyOwner->GetActorForwardVector();
	}
}


bool AGun::CanShoot() const
{
	bool bOwnerCanFire = m_MyOwner && m_MyOwner->CanFire();
	bool bStateOKToFire = ((m_CurrentState == EWeaponState::Idle) || (m_CurrentState == EWeaponState::Firing));
	bool bMagazineIsNotEmpty = true;
	/*if (m_LoadedMagazine)
	{
		bMagazineIsNotEmpty = m_LoadedMagazine->RoundsLeft() != 0;
	}*/	
	return (bOwnerCanFire && bStateOKToFire && bMagazineIsNotEmpty);
}


/////////////////////////////////////////////////////
void AGun::OnEquip(AHumanoid* NewOwner)
{
	SetOwningPawn(NewOwner);
	AttachMeshToPawn();
}


void AGun::OnUnequip(bool DropGun /*= false*/)
{
	if (DropGun)
	{
		DetachMeshFromPawn();
		SetOwningPawn(nullptr);
	}
	else
	{
		m_Mesh->SetVisibility(false, true);
		m_Mesh->SetSimulatePhysics(false);
		m_Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
}


/////////////////////////////////////////////////////
//void AGun::OnPickUp(AActor* NewOwner)
//{
//	SetOwningPawn(NewOwner);
//	AttachMeshToPawn();
//}
//
//
//void AGun::OnDrop()
//{
//	SetOwningPawn(nullptr);
//	DetachMeshFromPawn();
//}


/////////////////////////////////////////////////////
//UUserWidget* AGun::OnBeginTraceCastOver(APawn* TracingPawn)
//{
//	if (TracingPawn && TracingPawn->IsLocallyControlled())
//	{
//		return m_InfoWidget;
//	}
//	else
//	{
//		return nullptr;
//	}	
//}


void AGun::OnFire()
{
	UWorld* const World = GetWorld();
	if (CanShoot() && World)
	{
		FVector traceStartLocation;
		// Set up the trace starting location
		if (m_MyOwner->IsPlayerControlled())
		{
			traceStartLocation = Cast<APlayerCharacter>(m_MyOwner)->GetCameraLocation();
		}
		else
		{
			traceStartLocation = GetMuzzleLocation();
		}

		FVector lineTraceDirection = GetForwardCameraVector();
		FVector SpawnDirection = AdjustAimRotation(traceStartLocation, lineTraceDirection);
		SpawnDirection = FMath::VRandCone(SpawnDirection, m_SpreadHorizontal, m_SpreadVertical);
		
		FVector SpawnLocation = GetMuzzleLocation();

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		// spawn the projectile at the muzzle
		/*TSubclassOf<AProjectile> projectileClass = m_LoadedMagazine->GetProjectileClass(true);*/
		AProjectile* projectile = World->SpawnActor<AProjectile>(m_ProjectileToSpawn, SpawnLocation, SpawnDirection.Rotation(), ActorSpawnParams);

		MakeNoise(1.0f, m_MyOwner, SpawnLocation);

		if (m_FireSound) // try and play the sound if specified
		{
			UGameplayStatics::PlaySoundAtLocation(this, m_FireSound, GetActorLocation());
		}

		if (m_FireAnimation)	// try and play a firing animation if specified
		{
			UAnimInstance* AnimInstance;	// Get the animation object for the arms mesh
			if (m_MyOwner->IsPlayerControlled())
			{
				AnimInstance = m_MyOwner->GetMesh()->GetAnimInstance();
				if (AnimInstance)
				{
					AnimInstance->Montage_Play(m_FireAnimation, 1.f);
				}
			}
		}

		if (GetCooldownTime() > 0.0f && m_CurrentState == EWeaponState::Firing)
		{
			GetWorld()->GetTimerManager().SetTimer(m_WeaponCooldownTimer, this, &AGun::ContinousOnFire, GetCooldownTime(), false);
		}
	}
}


/////////////////////////////////////////////////////
void AGun::SetOwningPawn(AActor* NewOwner)
{
	AHumanoid* humanoid = Cast<AHumanoid>(NewOwner);
	if (GetOwner() != NewOwner)
	{
		Instigator = humanoid;
		SetOwner(humanoid);
		m_MyOwner = humanoid;
	}
}


/////////////////////////////////////////////////////
void AGun::AttachMeshToPawn()
{
	if (m_MyOwner)
	{
		DetachMeshFromPawn();

		FName AttachPoint = m_MyOwner->GetWeaponAttachPoint();
		USkeletalMeshComponent* PawnMesh = m_MyOwner->GetMesh();

		if (m_Mesh)
		{
			m_Mesh->AttachToComponent(PawnMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachPoint);
			m_Mesh->SetSimulatePhysics(false);
		}
	}
}


void AGun::DetachMeshFromPawn()
{
	if (m_Mesh)
	{
		m_Mesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		m_Mesh->SetSimulatePhysics(true);
	}
}


/////////////////////////////////////////////////////
float AGun::GetCooldownTime() const
{
	return m_Cooldown;
}


/////////////////////////////////////////////////////
float AGun::GetRoundsPerMinute() const
{
	return 60 / GetCooldownTime();
}


FVector AGun::GetMuzzleLocation() const
{
	FVector VecMuzzleLocation;
	FRotator MuzzleRotation;

	if (m_Mesh)
	{
		m_Mesh->GetSocketWorldLocationAndRotation(m_MuzzleAttachPoint, VecMuzzleLocation, MuzzleRotation);
	}
	return VecMuzzleLocation;
}