// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/Gun.h"
#include "Humanoid.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Enums/WeaponEnums.h"
#include "Enums/ItemEnums.h"
#include "Weapons/Projectile.h"
#include "Animation/AnimInstance.h"
#include "PlayerCharacter.h"
#include "CoopArena.h"
#include "Magazine.h"
#include "Projectile.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Camera/CameraComponent.h"


AGun::AGun()
{
	_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetUpMesh();

	_InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	SetUpInteractionVolume();

	_ZoomCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ZoomCamera"));
	_ZoomCamera->SetupAttachment(_Mesh, "Scope");
	_ZoomCamera->SetAutoActivate(true);

	_CurrentGunState = EWeaponState::Idle;

	_MuzzleAttachPoint = "Muzzle";

	_itemStats.type = EItemType::Weapon;
	_itemStats.itemClass = GetClass();

	_GunStats.WeaponType = EWEaponType::Rifle;	

	_BurstCount = 0;
	_SalvoCount = 0;
}


void AGun::SetUpMesh()
{
	_Mesh->SetCollisionObjectType(ECC_PhysicsBody);
	_Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	_Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	_Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	_Mesh->SetCollisionResponseToChannel(ECC_Interactable, ECR_Block);
	_Mesh->SetSimulatePhysics(true);
	_Mesh->CastShadow = true;
	_Mesh->SetCustomDepthStencilValue(253);
	RootComponent = _Mesh;
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
		SetCanBeInteractedWith_Implementation(false);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s was equipped but doesn't have an owner!"), *GetName());
	}
}


void AGun::OnUnequip(bool DropGun /*= false*/)
{
	_MyOwner->SetEquippedWeapon(nullptr);
	
	if (DropGun)
	{
		DetachMeshFromPawn();
		SetOwningPawn(nullptr);
		SetCanBeInteractedWith_Implementation(true);
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
			AnimInstance = _MyOwner->GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(_FireAnimation, 1.f);
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
		FName AttachPoint = _MyOwner->GetEquippedWeaponAttachPoint();
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
	
	bool bHasMag = false;
	if (_LoadedMagazine)
	{
		bHasMag = inventory->RemoveItem(_LoadedMagazine->GetItemStats(), 1.0f);
	}
	else
	{
		AMagazine* magObject = Cast<AMagazine>(_GunStats.UsableMagazineClass->GetDefaultObject(true));
		bHasMag = inventory->RemoveItem(magObject->GetItemStats(), 1.0f);
	}

	return bHasMag;
}


bool AGun::CheckIfOwnerHasMagazine() const
{
	UInventoryComponent* inventory = Cast<UInventoryComponent>(_MyOwner->GetComponentByClass(UInventoryComponent::StaticClass()));	
	if (inventory == nullptr)
	{
		return false;
	}

	if (_LoadedMagazine)
	{
		FItemStats& magStats = _LoadedMagazine->GetItemStats();
		return inventory->HasItem(magStats);
	}
	else
	{
		AMagazine* magObject = Cast<AMagazine>(_GunStats.UsableMagazineClass->GetDefaultObject(true));
		FItemStats& magStats = magObject->GetItemStats();
		return inventory->HasItem(magStats);
	}	
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
	else
	{
		DropMagazine();
		FTimerHandle reloadTH;
		FTimerDelegate lambda;
		lambda.BindLambda([this]
		{
			bool bHasAmmo = GetAmmoFromInventory();
			if (bHasAmmo)
			{
				AMagazine* newMag = SpawnNewMagazine();
				AttachMagazine(newMag);
			}
			FinishReloadWeapon();
		});

		GetWorld()->GetTimerManager().SetTimer(reloadTH, lambda, reloadTime, false);
	}
}


void AGun::StopReloading()
{
	if (_ReloadAnimation)
	{
		_MyOwner->StopAnimMontage(_ReloadAnimation);
	}
	FinishReloadWeapon();
}


void AGun::DropMagazine()
{
	if (_LoadedMagazine == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s carried by %s tried to drop it's magazine without having one attached!"), *GetName(), *GetOwner()->GetName());
		return;
	}

	_LoadedMagazine->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	_LoadedMagazine->SetSimulatePhysics(true);
	if (_LoadedMagazine->RoundsLeft() == _LoadedMagazine->GetCapacity())
	{
		_LoadedMagazine->SetCanBeInteractedWith_Implementation(true);
	}

	_LoadedMagazine = nullptr;
}


void AGun::FinishReloadWeapon()
{
	if (_LoadedMagazine)
	{
		_CurrentGunState = EWeaponState::Idle;
	}
	else
	{
		_CurrentGunState = EWeaponState::NoMagazine;
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

	AMagazine* newMagazine = SpawnNewMagazine();
	if (newMagazine)
	{
		AttachMagazine(newMagazine);
	}
}


/////////////////////////////////////////////////////
AMagazine* AGun::SpawnNewMagazine()
{
	if (_GunStats.UsableMagazineClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("'UsableMagazineClass' for %s is not set!"), *GetName());
		return nullptr;
	}
	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AMagazine* newMag = GetWorld()->SpawnActor<AMagazine>(_GunStats.UsableMagazineClass, GetActorLocation(), FRotator::ZeroRotator, spawnParams);
	newMag->SetCanBeInteractedWith_Implementation(false);

	return newMag;
}


void AGun::AttachMagazine(AMagazine* Magazine)
{
	if (_LoadedMagazine)
	{
		UE_LOG(LogTemp, Error, TEXT("%s tried to attach a magazine to %s while a magazine is loaded. Destroying existing magazine."), *_MyOwner->GetName(), *GetName());
		_LoadedMagazine->Destroy();
	}

	_LoadedMagazine = Magazine;
	_LoadedMagazine->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "Magazine");
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