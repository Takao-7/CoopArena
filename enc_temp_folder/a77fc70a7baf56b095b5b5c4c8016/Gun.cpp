// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/Gun.h"
#include "Humanoid.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "Enums/WeaponEnums.h"
#include "Enums/ItemEnums.h"
#include "Weapons/Projectile.h"
#include "Animation/AnimInstance.h"
#include "PlayerCharacter.h"
#include "CoopArena.h"
#include "Magazine.h"
#include "Projectile.h"
#include "InventoryComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Camera/CameraComponent.h"


AGun::AGun()
{
	_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	_Mesh->SetCollisionObjectType(ECC_PhysicsBody);
	_Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	_Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	_Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	_Mesh->SetCollisionResponseToChannel(ECC_Interactable, ECR_Block);
	_Mesh->SetSimulatePhysics(true);
	_Mesh->CastShadow = true;
	_Mesh->SetCustomDepthStencilValue(253);
	SetRootComponent(_Mesh);

	_InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("Interaction box"));
	_InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	_InteractionVolume->SetCollisionResponseToChannel(ECC_Interactable, ECR_Block);
	_InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	_InteractionVolume->SetupAttachment(RootComponent);

	_ZoomCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Zoom Camera"));
	_ZoomCamera->SetupAttachment(_Mesh, "Scope");
	_ZoomCamera->SetAutoActivate(false);

	_CurrentGunState = EWeaponState::Idle;

	_MuzzleAttachPoint = "Muzzle";

	_itemStats.type = EItemType::Weapon;
	_itemStats.itemClass = GetClass();

	_GunStats.WeaponType = EWEaponType::Rifle;	

	_BurstCount = 0;
	_SalvoCount = 0;
}


/////////////////////////////////////////////////////
void AGun::OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent)
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


/////////////////////////////////////////////////////
FVector AGun::AdjustAimRotation(FVector traceStart, FVector direction)
{
	FVector traceEnd = traceStart + direction * _GunStats.lineTraceRange;

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
		_Mesh->SetSimulatePhysics(false);
		_Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
}


/////////////////////////////////////////////////////
void AGun::OnFire()
{
	if (CanShoot())
	{
		_CurrentGunState = EWeaponState::Firing;
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
		SpawnDirection = ApplyWeaponSpread(SpawnDirection);		
		FVector SpawnLocation = GetMuzzleLocation();
		FTransform SpawnTransform = FTransform(SpawnDirection.Rotation(), SpawnLocation);

		TSubclassOf<AProjectile> projectileClass = _LoadedMagazine->GetProjectileClass();

		AProjectile* projectile = GetWorld()->SpawnActorDeferred<AProjectile>(projectileClass, SpawnTransform, GetOwner(), _MyOwner, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!projectile)
		{
			UE_LOG(LogTemp, Error, TEXT("Gun %s, owned by %s: No projectile spawned!"), *GetName(), *_MyOwner->GetName());
			return;
		}
		projectile->FinishSpawning(SpawnTransform);
		_LoadedMagazine->RemoveRound();

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

		if (_MuzzleFlash && !_SpawnedMuzzleFlashComponent)
		{
			_SpawnedMuzzleFlashComponent = UGameplayStatics::SpawnEmitterAttached(_MuzzleFlash, _Mesh, _MuzzleAttachPoint, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget);
		}

		if (CanRapidFire() && _CurrentGunState == EWeaponState::Firing)
		{
			GetWorld()->GetTimerManager().SetTimer(_WeaponCooldownTH, this, &AGun::ContinousOnFire, GetCooldownTime());
		}
	}
	else
	{
		OnStopFire();
	}
}


void AGun::ContinousOnFire()
{
	if (_CurrentFireMode == EFireMode::Auto && _CurrentGunState == EWeaponState::Firing)
	{
		_SalvoCount++;
		OnFire();
	}
	else if (_CurrentFireMode == EFireMode::Burst)
	{
		_BurstCount++;
		_SalvoCount++;
		if (_BurstCount < _GunStats.ShotsPerBurst)
		{
			OnFire();
		}
		else
		{
			_BurstCount = 0;
			_SalvoCount = 0;
			OnStopFire();
		}
	}
	else
	{
		OnStopFire();
	}
}



void AGun::OnStopFire()
{
	_CurrentGunState = EWeaponState::Idle;
	_SalvoCount = 0;
	if (_SpawnedMuzzleFlashComponent)
	{
		_SpawnedMuzzleFlashComponent->DestroyComponent();
		_SpawnedMuzzleFlashComponent = nullptr;
	}
}


/////////////////////////////////////////////////////
FVector AGun::ApplyWeaponSpread(FVector SpawnDirection)
{
	float spreadHorizontal = FMath::Clamp(_GunStats.SpreadHorizontal, 0.0f, _GunStats.MaxSpread);
	float spreadVertical = FMath::Clamp(_GunStats.SpreadVertical, 0.0f, _GunStats.MaxSpread);
	return FMath::VRandCone(SpawnDirection, spreadHorizontal, spreadVertical);
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
bool AGun::CanRapidFire() const
{
	return (_GunStats.FireModes.Contains(EFireMode::Burst) || _GunStats.FireModes.Contains(EFireMode::Auto)) && GetCooldownTime() > 0.0f;
}


bool AGun::CanShoot() const
{
	bool bOwnerCanFire = _MyOwner && _MyOwner->CanFire();
	bool bStateOKToFire = ((_CurrentGunState == EWeaponState::Idle) || (_CurrentGunState == EWeaponState::Firing));
	bool bMagazineIsNotEmpty = _LoadedMagazine && _LoadedMagazine->RoundsLeft() > 0;
	bool bHasProjectileToSpawn = _GunStats.UsableMagazineClass;
	return (bOwnerCanFire && bStateOKToFire && bMagazineIsNotEmpty && bHasProjectileToSpawn);
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
bool AGun::GetAmmoFromInventory()
{
	UInventoryComponent* inventory = Cast<UInventoryComponent>(_MyOwner->GetComponentByClass(UInventoryComponent::StaticClass()));
	if (inventory == nullptr)
	{
		return false;
	}
	return inventory->RemoveItemByClass(_GunStats.UsableMagazineClass);
}


bool AGun::CheckIfOwnerHasMagazine()
{
	UInventoryComponent* inventory = Cast<UInventoryComponent>(_MyOwner->GetComponentByClass(UInventoryComponent::StaticClass()));	
	if (inventory == nullptr)
	{
		return false;
	}
	return inventory->GetItemCountByClass(_GunStats.UsableMagazineClass);
}


/////////////////////////////////////////////////////
void AGun::ReloadWeapon()
{
	if (_CurrentGunState == EWeaponState::Reloading || !CheckIfOwnerHasMagazine())
	{
		return;
	}
	_CurrentGunState = EWeaponState::Reloading;
	float reloadTime = 3.0f; // Default reloading time, if there is no reload animation for some reason.
	FTimerHandle reloadTH;
	if (_ReloadAnimation)
	{
		UAnimInstance* AnimInstance;
		if (_MyOwner->IsPlayerControlled())
		{
			AnimInstance = _MyOwner->GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				reloadTime = AnimInstance->Montage_Play(_ReloadAnimation, 1.f);		
			}
		}
	}
	if (_ReloadSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, _ReloadSound, GetActorLocation());
	}
	GetWorld()->GetTimerManager().SetTimer(reloadTH, this, &AGun::FinishReloadWeapon, reloadTime);
}


void AGun::FinishReloadWeapon()
{
	if (GetAmmoFromInventory())
	{
		SpawnNewMagazine();
		_CurrentGunState = EWeaponState::Idle;
	}
	else
	{
		_CurrentGunState = EWeaponState::Blocked;
	}	
}


/////////////////////////////////////////////////////
void AGun::ToggleFireMode()
{
	_CurrentFireModePointer = (_CurrentFireModePointer + 1) % _GunStats.FireModes.Num();
	_CurrentFireMode = _GunStats.FireModes[_CurrentFireModePointer];
}


UMeshComponent* AGun::GetMesh() const
{
	return _Mesh;
}


UCameraComponent* AGun::GetZoomCamera() const
{
	return _ZoomCamera;
}


/////////////////////////////////////////////////////
void AGun::BeginPlay()
{
	Super::BeginPlay();

	_CurrentFireMode = _GunStats.FireModes[0];
	_CurrentFireModePointer = 0;

	SpawnNewMagazine();
}


/////////////////////////////////////////////////////
void AGun::SpawnNewMagazine()
{
	if (_LoadedMagazine)
	{
		_LoadedMagazine->Destroy();
	}
	
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	_LoadedMagazine = GetWorld()->SpawnActor<AMagazine>(_GunStats.UsableMagazineClass, GetActorLocation(), FRotator::ZeroRotator, spawnParams);
	if (_LoadedMagazine)
	{
		_LoadedMagazine->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}


/////////////////////////////////////////////////////
float AGun::GetCooldownTime() const
{
	return _GunStats.Cooldown;
}


float AGun::GetRoundsPerMinute() const
{
	return 60.0f / GetCooldownTime();
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