// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/Gun.h"
#include "Humanoid.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "CoopArena.h"
#include "Weapons/Projectile.h"
#include "Animation/AnimInstance.h"
#include "PlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"


AGun::AGun()
{
	_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	_Mesh->SetCollisionObjectType(ECC_PhysicsBody);
	_Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	_Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	_Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	_Mesh->SetSimulatePhysics(true);
	_Mesh->CastShadow = true;
	_Mesh->SetCustomDepthStencilValue(253);
	_Mesh->SetCollisionResponseToChannel(ECC_Item, ECR_Block);
	SetRootComponent(_Mesh);

	_CurrentState = EWeaponState::Idle;

	_MuzzleAttachPoint = "Muzzle";

	_itemStats.Type = EItemType::Weapon;
	_itemStats.Class = GetClass();

	_WeaponStats.WeaponType = EWEaponType::None;
}


void AGun::OnBeginInteract_Implementation(APawn* InteractingPawn)
{
	AHumanoid* character = Cast<AHumanoid>(InteractingPawn);
	if (character)
	{
		AGun* equippedGun = character->GetEquippedGun();
		if (equippedGun)
		{
			equippedGun->OnUnequip(true);
			FTimerHandle timerHandle;
			FTimerDelegate timerDelegate;
			timerDelegate.BindUFunction(this, FName("OnEquip"), character);
			GetWorld()->GetTimerManager().SetTimer(timerHandle, timerDelegate, 0.5f, false);
		}
		else
		{
			OnEquip(character);
		}		
	}	
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
	bool bHasProjectileToSpawn = _WeaponStats.ProjectileToSpawn;
	return (bOwnerCanFire && bStateOKToFire && bMagazineIsNotEmpty && bHasProjectileToSpawn);
}


/////////////////////////////////////////////////////
void AGun::OnEquip(AHumanoid* NewOwner)
{
	SetOwningPawn(NewOwner);
	AttachMeshToPawn();
	if (NewOwner)
	{
		NewOwner->SetEquippedWeapon(this);
	}
}


void AGun::OnUnequip(bool DropGun /*= false*/)
{
	_MyOwner->SetEquippedWeapon(nullptr);
	
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


/////////////////////////////////////////////////////
void AGun::OnFire()
{
	UWorld* const World = GetWorld();
	if (CanShoot() && World)
	{
		FVector traceStartLocation;
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
		SpawnDirection = FMath::VRandCone(SpawnDirection, _WeaponStats.SpreadHorizontal, _WeaponStats.SpreadVertical);
		
		FVector SpawnLocation = GetMuzzleLocation();

		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		AProjectile* projectile = World->SpawnActor<AProjectile>(_WeaponStats.ProjectileToSpawn, SpawnLocation, SpawnDirection.Rotation(), ActorSpawnParams);
		if (projectile)
		{
			projectile->_Instigator = GetOwner()->GetInstigatorController();
		}

		MakeNoise(1.0f, _MyOwner, SpawnLocation);

		if (_FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, _FireSound, GetActorLocation());
		}

		if (_FireAnimation)
		{
			UAnimInstance* AnimInstance;
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
void AGun::SetOwningPawn(AHumanoid* NewOwner)
{
	if (GetOwner() != NewOwner)
	{
		Instigator = NewOwner;
		SetOwner(NewOwner);
		_MyOwner = NewOwner;
	}
}


/////////////////////////////////////////////////////
void AGun::AttachMeshToPawn()
{
	if (_MyOwner)
	{
		FName AttachPoint = _MyOwner->GetWeaponAttachPoint();
		USkeletalMeshComponent* PawnMesh = _MyOwner->GetMesh();

		if (_Mesh)
		{
			_Mesh->SetSimulatePhysics(false);
			_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			_Mesh->SetCollisionObjectType(ECC_WorldDynamic);
			_Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
			_Mesh->AttachToComponent(PawnMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachPoint);
		}
	}
}


void AGun::DetachMeshFromPawn()
{
	if (_Mesh)
	{
		_Mesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		_Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		_Mesh->SetCollisionResponseToAllChannels(ECR_Block);
		_Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		_Mesh->SetSimulatePhysics(true);
	}
}


/////////////////////////////////////////////////////
float AGun::GetCooldownTime() const
{
	return _WeaponStats.Cooldown;
}


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