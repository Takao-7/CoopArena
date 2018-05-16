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
	_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	_Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	_Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	_Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	_Mesh->CastShadow = true;
	SetRootComponent(_Mesh);

	_CurrentState = EWeaponState::Idle;
}


FVector AGun::AdjustAimRotation(FVector traceStart, FVector direction)
{
	FVector traceEnd = traceStart + direction * _lineTraceRange;

	// Start the trace further away, in the given direction, so that we don't hit ourself.
	FVector adjustedTraceStart = traceStart + (direction * _MyOwner->GetCapsuleComponent()->GetScaledCapsuleRadius() * 1.3f);

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
	/*if (_CurrentFireMode == EFireMode::Burst)
	{
		if (_BurstCount > 0)
		{
			_BurstCount--;
			UseTool();
		}
		else
		{
			if (GetCooldownTime() == 0)
			{
				_CurrentFireMode = EFireMode::Single;
			}
			else
			{
				_CurrentFireMode = EFireMode::Auto;
			}
			_CurrentState = EWeaponState::Idle;
}
	}
	else if (_CurrentState == EWeaponState::Firing)
	{
		UseTool();
	}

	if (_BurstCount < 1)
	{
		_BurstCount = _ShotsPerBurst;
	}*/
}


FVector AGun::GetForwardCameraVector() const
{
	if (_MyOwner->IsPlayerControlled())
	{
		FVector CamPos;
		FRotator CamRot;
		Cast<APlayerController>(_MyOwner->Controller)->GetPlayerViewPoint(CamPos, CamRot);
		return CamRot.Vector();
	}
	else
	{
		return _MyOwner->GetActorForwardVector();
	}
}


bool AGun::CanShoot() const
{
	bool bOwnerCanFire = _MyOwner && _MyOwner->CanFire();
	bool bStateOKToFire = ((_CurrentState == EWeaponState::Idle) || (_CurrentState == EWeaponState::Firing));
	bool bMagazineIsNotEmpty = true;
	/*if (_LoadedMagazine)
	{
		bMagazineIsNotEmpty = _LoadedMagazine->RoundsLeft() != 0;
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
		_Mesh->SetVisibility(false, true);
		_Mesh->SetSimulatePhysics(false);
		_Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
}


void AGun::OnFire()
{
	UWorld* const World = GetWorld();
	if (CanShoot() && World)
	{
		FVector traceStartLocation;
		// Set up the trace starting location
		if (_MyOwner->IsPlayerControlled())
		{
			traceStartLocation = Cast<APlayerCharacter>(_MyOwner)->GetCameraLocation();
		}
		else
		{
			traceStartLocation = GetMuzzleLocation();
		}

		FVector lineTraceDirection = GetForwardCameraVector();
		FVector SpawnDirection = AdjustAimRotation(traceStartLocation, lineTraceDirection);
		SpawnDirection = FMath::VRandCone(SpawnDirection, _SpreadHorizontal, _SpreadVertical);
		
		FVector SpawnLocation = GetMuzzleLocation();

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		// spawn the projectile at the muzzle
		AProjectile* projectile = World->SpawnActor<AProjectile>(_ProjectileToSpawn, SpawnLocation, SpawnDirection.Rotation(), ActorSpawnParams);

		MakeNoise(1.0f, _MyOwner, SpawnLocation);

		if (_FireSound) // try and play the sound if specified
		{
			UGameplayStatics::PlaySoundAtLocation(this, _FireSound, GetActorLocation());
		}

		if (_FireAnimation)	// try and play a firing animation if specified
		{
			UAnimInstance* AnimInstance;	// Get the animation object for the arms mesh
			if (_MyOwner->IsPlayerControlled())
			{
				AnimInstance = _MyOwner->GetMesh()->GetAnimInstance();
				if (AnimInstance)
				{
					AnimInstance->Montage_Play(_FireAnimation, 1.f);
				}
			}
		}

		if (GetCooldownTime() > 0.0f && _CurrentState == EWeaponState::Firing)
		{
			GetWorld()->GetTimerManager().SetTimer(_WeaponCooldownTimer, this, &AGun::ContinousOnFire, GetCooldownTime(), false);
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
		_MyOwner = humanoid;
	}
}


/////////////////////////////////////////////////////
void AGun::AttachMeshToPawn()
{
	if (_MyOwner)
	{
		DetachMeshFromPawn();

		FName AttachPoint = _MyOwner->GetWeaponAttachPoint();
		USkeletalMeshComponent* PawnMesh = _MyOwner->GetMesh();

		if (_Mesh)
		{
			_Mesh->AttachToComponent(PawnMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachPoint);
			_Mesh->SetSimulatePhysics(false);
		}
	}
}


void AGun::DetachMeshFromPawn()
{
	if (_Mesh)
	{
		_Mesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		_Mesh->SetSimulatePhysics(true);
	}
}


/////////////////////////////////////////////////////
float AGun::GetCooldownTime() const
{
	return _Cooldown;
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

	if (_Mesh)
	{
		_Mesh->GetSocketWorldLocationAndRotation(_MuzzleAttachPoint, VecMuzzleLocation, MuzzleRotation);
	}
	return VecMuzzleLocation;
}