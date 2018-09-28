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
	m_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetUpMesh();

	_InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	SetUpInteractionVolume();

	_ZoomCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ZoomCamera"));
	_ZoomCamera->SetupAttachment(m_Mesh, "Scope");
	_ZoomCamera->SetAutoActivate(true);

	m_ForwardDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("ForwardDirection"));
	m_ForwardDirection->SetupAttachment(RootComponent);

	m_CurrentGunState = EWeaponState::Idle;

	_MuzzleAttachPoint = "Muzzle";

	_itemStats.type = EItemType::Weapon;
	_itemStats.itemClass = GetClass();

	m_GunStats.WeaponType = EWEaponType::Rifle;	

	_BurstCount = 0;
	_SalvoCount = 0;

	bReplicates = true;
}


void AGun::SetUpMesh()
{
	m_Mesh->SetCollisionObjectType(ECC_PhysicsBody);
	m_Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	m_Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	m_Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	m_Mesh->SetCollisionResponseToChannel(ECC_Interactable, ECR_Block);
	m_Mesh->SetSimulatePhysics(true);
	m_Mesh->CastShadow = true;
	m_Mesh->SetCustomDepthStencilValue(253);
	RootComponent = m_Mesh;
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
	FVector traceEnd = traceStart + direction * m_GunStats.lineTraceRange;

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

/////////////////////////////////////////////////////
void AGun::OnEquip(AHumanoid* NewOwner)
{
	SetOwningPawn(NewOwner);
	AttachMeshToPawn();
	IInteractable::Execute_SetCanBeInteractedWith(this, false);
	if (NewOwner == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s was equipped but doesn't have an owner!"), *GetName());
	}
}

/////////////////////////////////////////////////////
void AGun::OnUnequip(bool DropGun /*= false*/)
{
	m_MyOwner->SetEquippedWeapon(nullptr);
	
	if (DropGun)
	{
		DetachMeshFromPawn();
		SetOwningPawn(nullptr);
		SetCanBeInteractedWith_Implementation(true);
		if (HasAuthority())
		{
			SetReplicateMovement(true);
		}
	}
	else
	{
		m_Mesh->SetSimulatePhysics(false);
		m_Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
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
		m_CurrentGunState = EWeaponState::Firing;
		
		FVector spawnDirection;
		if (m_MyOwner->IsAiming() || !m_MyOwner->IsPlayerControlled())
		{
			spawnDirection = m_ForwardDirection->GetForwardVector();
		}
		else
		{
			const FVector lineTraceStartLocation = Cast<APlayerCharacter>(m_MyOwner)->GetCameraLocation();
			const FVector lineTraceDirection = GetForwardCameraVector();
			spawnDirection = AdjustAimRotation(lineTraceStartLocation, lineTraceDirection);
		}

		const FVector spawnLocation = GetMuzzleLocation();
		const FTransform spawnTransform = FTransform(spawnDirection.ToOrientationRotator(), spawnLocation);
		Server_OnFire(m_CurrentFireMode, spawnTransform);

		ApplyWeaponSpread();

		if (CanRapidFire() && m_CurrentGunState == EWeaponState::Firing)
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
	if (m_CurrentFireMode == EFireMode::Auto && m_CurrentGunState == EWeaponState::Firing)
	{
		_SalvoCount++;
		OnFire();
	}
	else if (m_CurrentFireMode == EFireMode::Burst)
	{
		_BurstCount++;
		_SalvoCount++;
		if (_BurstCount < m_GunStats.ShotsPerBurst)
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
	m_CurrentGunState = EWeaponState::Idle;
	_SalvoCount = 0;
	Server_OnStopFire();
}

/////////////////////////////////////////////////////
void AGun::ApplyWeaponSpread()
{
	const float spreadHorizontal = FMath::RandRange(-m_GunStats.SpreadHorizontal, m_GunStats.SpreadHorizontal);

	FRotator newControlRotation = m_MyOwner->GetControlRotation();
	newControlRotation += FRotator(m_GunStats.SpreadVertical, spreadHorizontal, 0.0f);
	m_MyOwner->GetController()->SetControlRotation(newControlRotation);
}

/////////////////////////////////////////////////////
void AGun::SetOwningPawn(AHumanoid* NewOwner)
{
	if(NewOwner != m_MyOwner)
	{
		Instigator = NewOwner;
		SetOwner(NewOwner);
		m_MyOwner = NewOwner;
	}
}

/////////////////////////////////////////////////////
bool AGun::CanRapidFire() const
{
	return (m_GunStats.FireModes.Contains(EFireMode::Burst) || m_GunStats.FireModes.Contains(EFireMode::Auto)) && GetCooldownTime() > 0.0f;
}

/////////////////////////////////////////////////////
bool AGun::CanShoot() const
{
	bool bOwnerCanFire = m_MyOwner && m_MyOwner->CanFire();
	bool bStateOKToFire = ((m_CurrentGunState == EWeaponState::Idle) || (m_CurrentGunState == EWeaponState::Firing));
	bool bMagazineIsNotEmpty = _LoadedMagazine && _LoadedMagazine->RoundsLeft() > 0;
	bool bHasProjectileToSpawn = m_GunStats.UsableMagazineClass;
	return (bOwnerCanFire && bStateOKToFire && bMagazineIsNotEmpty && bHasProjectileToSpawn);
}

/////////////////////////////////////////////////////
void AGun::AttachMeshToPawn()
{
	if (m_MyOwner)
	{
		FName AttachPoint = m_MyOwner->GetEquippedWeaponAttachPoint();
		USkeletalMeshComponent* PawnMesh = m_MyOwner->GetMesh();

		if (m_Mesh)
		{
			m_Mesh->SetSimulatePhysics(false);
			m_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			m_Mesh->SetCollisionObjectType(ECC_WorldDynamic);
			m_Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
			m_Mesh->AttachToComponent(PawnMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachPoint);
		}
	}
}

/////////////////////////////////////////////////////
void AGun::DetachMeshFromPawn()
{
	if (m_Mesh)
	{
		m_Mesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		m_Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		m_Mesh->SetCollisionResponseToAllChannels(ECR_Block);
		m_Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		m_Mesh->SetSimulatePhysics(true);
	}
}

/////////////////////////////////////////////////////
bool AGun::GetAmmoFromInventory()
{
	UInventoryComponent* inventory = Cast<UInventoryComponent>(m_MyOwner->GetComponentByClass(UInventoryComponent::StaticClass()));
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
		AMagazine* magObject = Cast<AMagazine>(m_GunStats.UsableMagazineClass->GetDefaultObject(true));
		bHasMag = inventory->RemoveItem(magObject->GetItemStats(), 1.0f);
	}

	return bHasMag;
}

/////////////////////////////////////////////////////
bool AGun::CheckIfOwnerHasMagazine() const
{
	UInventoryComponent* inventory = Cast<UInventoryComponent>(m_MyOwner->GetComponentByClass(UInventoryComponent::StaticClass()));	
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
		AMagazine* magObject = Cast<AMagazine>(m_GunStats.UsableMagazineClass->GetDefaultObject(true));
		FItemStats& magStats = magObject->GetItemStats();
		return inventory->HasItem(magStats);
	}	
}


/////////////////////////////////////////////////////
void AGun::OnAnimNotify_AttachMagToHand()
{
	m_MyOwner->GrabItem(_LoadedMagazine, true);
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
	
	m_ItemToGrab = SpawnNewMagazine(m_MyOwner->GetItemOffset(false));
	OnItemGrab();
}

void AGun::OnAnimNotify_AttachMagToGun()
{
	AMagazine* magInHand = Cast<AMagazine>(m_MyOwner->GetItemInHand());
	if (HasAuthority())
	{
		m_MyOwner->Multicast_ClearItemInHand();
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
	if (m_CurrentGunState == EWeaponState::Reloading || !CheckIfOwnerHasMagazine())
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
		m_MyOwner->GrabItem(m_ItemToGrab, true);
		_LoadedMagazine = nullptr;
	}
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
	if (m_CurrentGunState == EWeaponState::Reloading || !CheckIfOwnerHasMagazine())
	{
		return;
	}
	m_CurrentGunState = EWeaponState::Reloading;
	float reloadTime = 3.0f; // Default reloading time, if there is no reload animation for some reason.
	if (_ReloadAnimation)
	{
		UAnimInstance* AnimInstance;
		AnimInstance = m_MyOwner->GetMesh()->GetAnimInstance();
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
				FTransform magazineLocation = GetMesh()->GetSocketTransform("Magazine");
				AMagazine* newMag = SpawnNewMagazine(magazineLocation);
				AttachMagazine(newMag);
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
	AnimInstance = m_MyOwner->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(_ReloadAnimation, 1.f, EMontagePlayReturnType::MontageLength, 0.0f, false);
	}
}

/////////////////////////////////////////////////////
void AGun::Multicast_StopReloading_Implementation()
{
	if (_ReloadAnimation)
	{
		m_MyOwner->StopAnimMontage(_ReloadAnimation);
	}
	FinishReloadWeapon();
}

/////////////////////////////////////////////////////
void AGun::DropMagazine()
{
	AMagazine* magazineInHand = Cast<AMagazine>(m_MyOwner->GetItemInHand());
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
}


void AGun::FinishReloadWeapon()
{
	if (_LoadedMagazine)
	{
		m_CurrentGunState = EWeaponState::Idle;
	}
	else
	{
		m_CurrentGunState = EWeaponState::NoMagazine;
	}	
}


/////////////////////////////////////////////////////
void AGun::ToggleFireMode()
{
	_CurrentFireModePointer = (_CurrentFireModePointer + 1) % m_GunStats.FireModes.Num();
	m_CurrentFireMode = m_GunStats.FireModes[_CurrentFireModePointer];
}


UMeshComponent* AGun::GetMesh() const
{
	return m_Mesh;
}


UCameraComponent* AGun::GetZoomCamera() const
{
	return _ZoomCamera;
}


/////////////////////////////////////////////////////
void AGun::BeginPlay()
{
	Super::BeginPlay();

	m_CurrentFireMode = m_GunStats.FireModes[0];
	_CurrentFireModePointer = 0;

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
	}
}

/////////////////////////////////////////////////////
AMagazine* AGun::SpawnNewMagazine(const FTransform& SpawnTransform)
{
	if (!HasAuthority())
	{
		return nullptr;
	}

	if (m_GunStats.UsableMagazineClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("'UsableMagazineClass' for %s is not set!"), *GetName());
		return nullptr;
	}
	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	params.Owner = this;
	params.Instigator = GetInstigator();
	return GetWorld()->SpawnActor<AMagazine>(m_GunStats.UsableMagazineClass, SpawnTransform, params);
}

/////////////////////////////////////////////////////
void AGun::AttachMagazine(AMagazine* Magazine)
{	
	if (Magazine == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' tried to attach a magazine that is nullptr!"), *GetName());
		return;
	}
		
	IInteractable::Execute_SetCanBeInteractedWith(Magazine, false);
	Magazine->ShouldSimulatePhysics(false);
	Magazine->AttachToActor(this, FAttachmentTransformRules::SnapToTargetIncludingScale, "Magazine");
	_LoadedMagazine = Magazine;
}

/////////////////////////////////////////////////////
float AGun::GetCooldownTime() const
{
	return m_GunStats.Cooldown;
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

	if (m_Mesh)
	{
		m_Mesh->GetSocketWorldLocationAndRotation(_MuzzleAttachPoint, VecMuzzleLocation, MuzzleRotation);
	}
	return VecMuzzleLocation;
}

/////////////////////////////////////////////////////
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
	m_CurrentGunState = EWeaponState::Idle;
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
	m_MyOwner = NewOwner;
}

/////////////////////////////////////////////////////
void AGun::Server_OnFire_Implementation(EFireMode FireMode, FTransform SpawnTransform)
{
	if (CanShoot())
	{
		m_CurrentFireMode = FireMode;
	
		TSubclassOf<AProjectile> projectileClass = _LoadedMagazine->GetProjectileClass();
		FActorSpawnParameters params;
		params.Owner = GetOwner();
		params.Instigator = GetInstigator();
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		AProjectile* projectile = GetWorld()->SpawnActor<AProjectile>(projectileClass, SpawnTransform, params);
		if (!projectile)
		{
			UE_LOG(LogTemp, Error, TEXT("Gun %s, owned by %s: No projectile spawned!"), *GetName(), *m_MyOwner->GetName());
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
			_SpawnedMuzzleFlashComponent = UGameplayStatics::SpawnEmitterAttached(_MuzzleFlash, m_Mesh, _MuzzleAttachPoint);
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
		AnimInstance = m_MyOwner->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(_FireAnimation, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, false);
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