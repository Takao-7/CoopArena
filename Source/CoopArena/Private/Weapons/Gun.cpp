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
#include "UnrealNetwork.h"


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

	bReplicates = true;
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
			equippedGun->Multicast_OnUnequip(true);

			FTimerDelegate timerDelegate;
			FTimerHandle timerHandle;
			timerDelegate.BindLambda([this, character]
			{
				character->SetWeaponToEquip(this);
			});
			GetWorld()->GetTimerManager().SetTimer(timerHandle, timerDelegate, 0.5f, false);
		}
		else
		{
			character->SetWeaponToEquip(this);
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
	IInteractable::Execute_SetCanBeInteractedWith(this, false);
	if (NewOwner == nullptr)
	{
<<<<<<< HEAD
		NewOwner->SetEquippedWeapon(this);
		SetCanBeInteractedWith_Implementation(false);
	}
	else
	{
=======
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
		UE_LOG(LogTemp, Error, TEXT("%s was equipped but doesn't have an owner!"), *GetName());
	}
}

/////////////////////////////////////////////////////
void AGun::OnUnequip(bool DropGun /*= false*/)
{
	_MyOwner->SetEquippedWeapon(nullptr);
	
	if (DropGun)
	{
		DetachMeshFromPawn();
		SetOwningPawn(nullptr);
		SetCanBeInteractedWith_Implementation(true);
<<<<<<< HEAD
=======
		if (HasAuthority())
		{
			SetReplicateMovement(true);
		}
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
	}
	else
	{
		_Mesh->SetSimulatePhysics(false);
		_Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		if (HasAuthority())
		{
			SetReplicateMovement(false);
		}
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
<<<<<<< HEAD

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
=======
		Server_OnFire(_CurrentFireMode, SpawnTransform);
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8

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

/////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////
void AGun::OnStopFire()
{
	_CurrentGunState = EWeaponState::Idle;
	_SalvoCount = 0;
	Server_OnStopFire();
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
	if(NewOwner != _MyOwner)
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

/////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////
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
	if (inventory == nullptr || !HasAuthority())
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
<<<<<<< HEAD

	return bHasMag;
}
=======
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8

	return bHasMag;
}

<<<<<<< HEAD
=======
/////////////////////////////////////////////////////
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
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
<<<<<<< HEAD
=======
}


/////////////////////////////////////////////////////
void AGun::OnAnimNotify_AttachMagToHand()
{
	_MyOwner->GrabItem(_LoadedMagazine, true);
	_LoadedMagazine = nullptr;
}

void AGun::OnAnimNotify_DropMagazine()
{
	DropMagazine();	
}

void AGun::OnAnimNotify_CheckForAmmo()
{
	if (HasAuthority())
	{
		bool bHasMagazine = CheckIfOwnerHasMagazine();
		if (!bHasMagazine)
		{
			Multicast_StopReloading();
		}
	}
}

void AGun::OnAnimNotify_SpawnNewMag()
{
	if (!HasAuthority())
	{
		return;
	}

	bool bHasAmmo = GetAmmoFromInventory();
	if (!bHasAmmo)
	{
		UE_LOG(LogTemp, Error, TEXT("%s dosn't has any magazines in his inventory, but tried to spawn a new one!"), *GetOwner()->GetName());
		return;
	}
	
	m_ItemToGrab = SpawnNewMagazine(_MyOwner->GetItemOffset(false));
	OnItemGrab();
}

void AGun::OnAnimNotify_AttachMagToGun()
{
	AMagazine* magInHand = Cast<AMagazine>(_MyOwner->GetItemInHand());
	if (HasAuthority())
	{
		_MyOwner->Multicast_ClearItemInHand();
	}
	AttachMagazine(magInHand);	
}

void AGun::OnAnimNotify_FinishReloading()
{
	FinishReloadWeapon();	
}

/////////////////////////////////////////////////////
void AGun::Reload()
{
	if (_CurrentGunState == EWeaponState::Reloading || !CheckIfOwnerHasMagazine())
	{
		return;
	}

	Server_Reload();
}

/////////////////////////////////////////////////////
void AGun::OnItemGrab()
{
	if (m_ItemToGrab)
	{
		_MyOwner->GrabItem(m_ItemToGrab, true);
		_LoadedMagazine = nullptr;
	}
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
}

/////////////////////////////////////////////////////
void AGun::OnMagAttached()
{
	if (m_MagToAttach)
	{
		AttachMagazine(m_MagToAttach);
	}
}

/////////////////////////////////////////////////////
void AGun::Multicast_OnUnequip_Implementation(bool bDropGun)
{
	OnUnequip(true);
}

/////////////////////////////////////////////////////
void AGun::Server_Reload_Implementation()
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
		AnimInstance = _MyOwner->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			reloadTime = _ReloadAnimation->GetSectionLength(0);
			Multicast_PlayReloadAnimation();
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
<<<<<<< HEAD
				reloadTime = AnimInstance->Montage_Play(_ReloadAnimation, 1.f);
=======
				FTransform magazineLocation = GetMesh()->GetSocketTransform("Magazine");
				AMagazine* newMag = SpawnNewMagazine(magazineLocation);
				AttachMagazine(newMag);
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
			}
			FinishReloadWeapon();
		});

		GetWorld()->GetTimerManager().SetTimer(reloadTH, lambda, reloadTime, false);
	}
}

bool AGun::Server_Reload_Validate()
{
	return true;
}

/////////////////////////////////////////////////////
void AGun::Multicast_PlayReloadAnimation_Implementation()
{
	UAnimInstance* AnimInstance;	
	AnimInstance = _MyOwner->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(_ReloadAnimation, 1.f);
	}
}

/////////////////////////////////////////////////////
void AGun::Multicast_StopReloading_Implementation()
{
	if (_ReloadAnimation)
	{
		_MyOwner->StopAnimMontage(_ReloadAnimation);
	}
<<<<<<< HEAD
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
=======
	FinishReloadWeapon();
}

/////////////////////////////////////////////////////
void AGun::DropMagazine()
{
	AMagazine* magazineInHand = Cast<AMagazine>(_MyOwner->GetItemInHand());
	if (magazineInHand == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s carried by %s tried to drop it's magazine without having one in his hand!"), *GetName(), *GetOwner()->GetName());
		return;
	}

	magazineInHand->OnDrop();

	if (magazineInHand->RoundsLeft() < magazineInHand->GetCapacity())
	{
		IInteractable::Execute_SetCanBeInteractedWith(magazineInHand, false);
	}
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
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

<<<<<<< HEAD
	AMagazine* newMagazine = SpawnNewMagazine();
	if (newMagazine)
	{
		AttachMagazine(newMagazine);
=======
	if(HasAuthority())
	{
		AHumanoid* humanoidOwner = Cast<AHumanoid>(GetInstigator());
		if (humanoidOwner)
		{
			Multicast_RepMyOwner(humanoidOwner);
		}
		
		FTransform spawnTransform = GetMesh()->GetSocketTransform("Magazine", RTS_World);
		m_MagToAttach = SpawnNewMagazine(spawnTransform);
		AttachMagazine(m_MagToAttach);
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
	}
}

/////////////////////////////////////////////////////
<<<<<<< HEAD
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
=======
AMagazine* AGun::SpawnNewMagazine(const FTransform& SpawnTransform)
{
	if (!HasAuthority())
	{
		return nullptr;
	}

	if (_GunStats.UsableMagazineClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("'UsableMagazineClass' for %s is not set!"), *GetName());
		return nullptr;
	}
	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	params.Owner = this;
	params.Instigator = GetInstigator();
	return GetWorld()->SpawnActor<AMagazine>(_GunStats.UsableMagazineClass, SpawnTransform, params);
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
}

/////////////////////////////////////////////////////
void AGun::AttachMagazine(AMagazine* Magazine)
{	
	IInteractable::Execute_SetCanBeInteractedWith(Magazine, false);
	Magazine->ShouldSimulatePhysics(false);
	Magazine->AttachToActor(this, FAttachmentTransformRules::SnapToTargetIncludingScale, "Magazine");
	_LoadedMagazine = Magazine;
}

/////////////////////////////////////////////////////
float AGun::GetCooldownTime() const
{
	return _GunStats.Cooldown;
}

/////////////////////////////////////////////////////
float AGun::GetRoundsPerMinute() const
{
	return 60.0f / GetCooldownTime();
}

/////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////
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


/////////////////////////////////////////////////////
					/* Networking */
/////////////////////////////////////////////////////
void AGun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGun, m_ItemToGrab);
	DOREPLIFETIME(AGun, m_MagToAttach);
}

/////////////////////////////////////////////////////
void AGun::Server_OnStopFire_Implementation()
{
	_CurrentGunState = EWeaponState::Idle;
	_SalvoCount = 0;
	Multicast_HandleMuzzleFlash(false);
}

/////////////////////////////////////////////////////
bool AGun::Server_OnStopFire_Validate()
{
	return true;
}

/////////////////////////////////////////////////////
void AGun::Multicast_RepMyOwner_Implementation(AHumanoid* NewOwner)
{
	_MyOwner = NewOwner;
}

/////////////////////////////////////////////////////
void AGun::Server_OnFire_Implementation(EFireMode FireMode, FTransform SpawnTransform)
{
	if (CanShoot())
	{
		_CurrentFireMode = FireMode;
	
		TSubclassOf<AProjectile> projectileClass = _LoadedMagazine->GetProjectileClass();
		FActorSpawnParameters params;
		params.Owner = GetOwner();
		params.Instigator = GetInstigator();
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		AProjectile* projectile = GetWorld()->SpawnActor<AProjectile>(projectileClass, SpawnTransform, params);
		if (!projectile)
		{
			UE_LOG(LogTemp, Error, TEXT("Gun %s, owned by %s: No projectile spawned!"), *GetName(), *_MyOwner->GetName());
			return;
		}
		_LoadedMagazine->RemoveRound();

		MakeNoise(1.0f, GetInstigator(), SpawnTransform.GetLocation());
		Multicast_PlayFireSound();
		Multicast_PlayFireAnimation();
		Multicast_HandleMuzzleFlash(true);
	}
	else
	{
		Server_OnStopFire();
	}	
}

bool AGun::Server_OnFire_Validate(EFireMode FireMode, FTransform SpawnTransform)
{
	return true;
}

/////////////////////////////////////////////////////
void AGun::Multicast_HandleMuzzleFlash_Implementation(bool bSpawnMuzzleFlash)
{
	if(bSpawnMuzzleFlash)
	{
		if (_MuzzleFlash && _SpawnedMuzzleFlashComponent == nullptr)
		{
			_SpawnedMuzzleFlashComponent = UGameplayStatics::SpawnEmitterAttached(_MuzzleFlash, _Mesh, _MuzzleAttachPoint);
		}
	}
	else
	{
		if (_SpawnedMuzzleFlashComponent)
		{
			_SpawnedMuzzleFlashComponent->DeactivateSystem();
			_SpawnedMuzzleFlashComponent = nullptr;
		}
	}
}

/////////////////////////////////////////////////////
void AGun::Multicast_PlayFireAnimation_Implementation()
{
	if (_FireAnimation)
	{
		UAnimInstance* AnimInstance;
		AnimInstance = _MyOwner->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(_FireAnimation, 1.f);
		}
	}
}

/////////////////////////////////////////////////////
void AGun::Multicast_PlayFireSound_Implementation()
{
	if (_FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, _FireSound, GetActorLocation());
	}
}