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
#include "Components/SimpleInventory.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Camera/CameraComponent.h"
#include "UnrealNetwork.h"
#include "SoundNodeLocalPlayer.h"
#include "AudioThread.h"
#include "Bot.h"


AGun::AGun()
{
	_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetUpMesh();

	_InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	SetUpInteractionVolume();

	_ZoomCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ZoomCamera"));
	_ZoomCamera->SetupAttachment(_Mesh, "Scope");
	_ZoomCamera->SetAutoActivate(true);

	_EquipOffset = CreateDefaultSubobject<USceneComponent>(TEXT("Offset"));
	_EquipOffset->SetupAttachment(RootComponent);

	_LeftHandPosition = CreateDefaultSubobject<USceneComponent>(TEXT("Left hand position"));
	_LeftHandPosition->SetupAttachment(RootComponent);

	_CurrentGunState = EWeaponState::Idle;

	_MuzzleAttachPoint = "Muzzle";
	_ShellEjectionPoint = "CaseEjection";

	_itemStats.type = EItemType::Weapon;
	_itemStats.itemClass = GetClass();

	_GunStats.WeaponType = EWEaponType::Rifle;	

	_BurstCount = 0;
	_SalvoCount = 0;

	_VerticalSpreadToApply = 0.0f;
	_HorizontalSpreadToApply = 0.0f;
	_GunStats.KickbackSpeed = 10.0f;

	_DespawnTime = 30.0f;

	bReplicates = true;
	bReplicateMovement = false;
	PrimaryActorTick.bCanEverTick = true;
}

/////////////////////////////////////////////////////
void AGun::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ApplyWeaponSpread(DeltaSeconds);
}

/////////////////////////////////////////////////////
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
void AGun::HandleOnDestroyed(AActor* DestroyedActor)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(DestroyedActor);
	if (_LoadedMagazine)
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(_LoadedMagazine);
	}
}

/////////////////////////////////////////////////////
void AGun::OnBeginInteract_Implementation(APawn* InteractingPawn, UPrimitiveComponent* HitComponent)
{
	AHumanoid* humanoid = Cast<AHumanoid>(InteractingPawn);
	if (humanoid)
	{
		AGun* equippedGun = humanoid->GetEquippedGun();
		if (equippedGun)
		{
			humanoid->UnequipWeapon(false, true);
			humanoid->AttachWeaponToHolster(equippedGun);
			humanoid->SetHolsterWeapon(equippedGun);
		}
		humanoid->EquipWeapon(this);
	}
}

/////////////////////////////////////////////////////
FVector AGun::AdjustAimRotation(FVector traceStart, FVector direction)
{
	FVector traceEnd = traceStart + direction * _GunStats.LineTraceRange;

	// Start the trace further away, in the given direction, so that we don't hit ourself.
	FVector adjustedTraceStart = traceStart + (direction * _MyOwner->GetCapsuleComponent()->GetScaledCapsuleRadius() * 1.3f);

	FHitResult Hit;
	GetWorld()->LineTraceSingleByChannel(Hit, adjustedTraceStart, traceEnd, ECollisionChannel::ECC_Camera);

	if (!Hit.IsValidBlockingHit())
	{
		return direction;
	}

	FVector adjustetDirection = Hit.Location - GetMuzzleTransform().GetLocation();
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

	AddToAudioActorCache();
}

/////////////////////////////////////////////////////
void AGun::AddToAudioActorCache()
{
	const APlayerController* PC = _MyOwner ? Cast<APlayerController>(_MyOwner->GetController()) : nullptr;
	const bool bLocallyControlled = (PC ? PC->IsLocalController() : false);
	const uint32 UniqueID = GetUniqueID();
	FAudioThread::RunCommandOnAudioThread([UniqueID, bLocallyControlled]()
	{
		USoundNodeLocalPlayer::GetLocallyControlledActorCache().Add(UniqueID, bLocallyControlled);
	});
}

void AGun::RemoveFromAudioActorCache()
{
	const uint32 UniqueID = GetUniqueID();
	FAudioThread::RunCommandOnAudioThread([UniqueID]()
	{
		USoundNodeLocalPlayer::GetLocallyControlledActorCache().Remove(UniqueID);
	});
}

/////////////////////////////////////////////////////
void AGun::Unequip(bool bDropGun /*= false*/, bool bRequestMulticast /*= true*/)
{	
	if (bRequestMulticast)
	{
		if (HasAuthority())
		{
			Unequip_Multicast(bDropGun);
		}
		else
		{
			Unequip_Server(bDropGun);
		}
	}
	else
	{
		if (bDropGun)
		{
			const bool bIsPlayerControlled = Instigator && Instigator->IsPlayerControlled();
			if (HasAuthority() && (!bIsPlayerControlled || Instigator == nullptr))
			{
				SetLifeSpan(_DespawnTime);
				if (_LoadedMagazine)
				{
					_LoadedMagazine->SetLifeSpan(_DespawnTime);
				}
			}
			DetachMeshFromPawn();
			SetOwningPawn(nullptr);
			SetCanBeInteractedWith_Implementation(true);
			SetReplicateMovement(true);
		}
		else
		{
			_Mesh->SetSimulatePhysics(false);
			_Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
			SetReplicateMovement(false);
		}

		RemoveFromAudioActorCache();
	}
}

void AGun::Unequip_Server_Implementation(bool bDropGun)
{
	Unequip_Multicast(bDropGun);
}

bool AGun::Unequip_Server_Validate(bool bDropGun)
{
	return true;
}

void AGun::Unequip_Multicast_Implementation(bool bDropGun)
{
	Unequip(bDropGun, false);
}

/////////////////////////////////////////////////////
void AGun::FireGun()
{
	if (CanShoot())
	{
		_CurrentGunState = EWeaponState::Firing;		

		FTransform muzzleTransform = GetMuzzleTransform();
		
		FRotator spawnDirection = FRotator::ZeroRotator;
		if (_MyOwner->IsAiming() || _MyOwner->IsPlayerControlled() == false)
		{
			spawnDirection = muzzleTransform.GetRotation().Rotator();
		}
		else
		{
			const FVector lineTraceStartLocation = Cast<APlayerCharacter>(_MyOwner)->GetMesh()->GetSocketLocation("head");
			const FVector lineTraceDirection = GetForwardCameraVector();
			spawnDirection = AdjustAimRotation(lineTraceStartLocation, lineTraceDirection).ToOrientationRotator();
		}

		ABot* owner = GetOwnerAsBot();
		if (owner)
		{
			FVector2D additionalSpread = owner->GetAdditionalSpread();
			spawnDirection.Yaw += FMath::FRandRange(0.0f, additionalSpread.X);
			spawnDirection.Pitch += FMath::FRandRange(0.0f, additionalSpread.Y);		
		}

		const FVector spawnLocation = muzzleTransform.GetLocation();
		const FTransform spawnTransform = FTransform(spawnDirection, spawnLocation);
		OnFire_Server(_CurrentFireMode, spawnTransform);

		AddWeaponSpread();

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
FORCEINLINE ABot* AGun::GetOwnerAsBot() const
{
	return Cast<ABot>(_MyOwner);
}

/////////////////////////////////////////////////////
void AGun::ContinousOnFire()
{
	if (_CurrentFireMode == EFireMode::Auto && _CurrentGunState == EWeaponState::Firing)
	{
		_SalvoCount++;
		FireGun();
	}
	else if (_CurrentFireMode == EFireMode::Burst)
	{
		_BurstCount++;
		_SalvoCount++;
		if (_BurstCount < _GunStats.ShotsPerBurst)
		{
			FireGun();
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
void AGun::AddWeaponSpread()
{
	_HorizontalSpreadToApply += FMath::RandRange(-_GunStats.SpreadHorizontal, _GunStats.SpreadHorizontal);
	_VerticalSpreadToApply += _GunStats.SpreadVertical;
	PrimaryActorTick.SetTickFunctionEnable(true);
}

void AGun::ApplyWeaponSpread(float DeltaSeconds)
{
	if ((_VerticalSpreadToApply == 0.0f && _HorizontalSpreadToApply == 0.0f) || _MyOwner == nullptr || (_MyOwner && _MyOwner->GetController() == nullptr))
	{
		PrimaryActorTick.SetTickFunctionEnable(false);
		return;
	}
	
	FRotator newControlRotation = _MyOwner->GetControlRotation();

	if (_VerticalSpreadToApply)
	{
		const float currentPitch = newControlRotation.Pitch;
		const float newPitch = FMath::FInterpTo(currentPitch, currentPitch + _VerticalSpreadToApply, DeltaSeconds, _GunStats.KickbackSpeed);
		_VerticalSpreadToApply -= newPitch - currentPitch;
		if (FMath::IsNearlyZero(_VerticalSpreadToApply) || _VerticalSpreadToApply < 0.0f)
		{
			_VerticalSpreadToApply = 0.0f;
		}

		newControlRotation.Pitch = newPitch;
	}
	if (_HorizontalSpreadToApply)
	{
		const float currentYaw = newControlRotation.Yaw;
		const float newYaw = FMath::FInterpTo(currentYaw, currentYaw + _HorizontalSpreadToApply, DeltaSeconds, _GunStats.KickbackSpeed);
		_HorizontalSpreadToApply -= newYaw - currentYaw;
		if (FMath::IsNearlyZero(_HorizontalSpreadToApply) || _HorizontalSpreadToApply < 0.0f)
		{
			_HorizontalSpreadToApply = 0.0f;
		}

		newControlRotation.Yaw = newYaw;
	}

	_MyOwner->GetController()->SetControlRotation(newControlRotation);
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

	if (HasAuthority() && NewOwner)
	{
		SetLifeSpan(0);
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
	if (_MyOwner && _Mesh)
	{
		FName attachPoint = _MyOwner->GetEquippedWeaponAttachPoint();
		USkeletalMeshComponent* PawnMesh = _MyOwner->GetMesh();

		_Mesh->SetSimulatePhysics(false);
		_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		_Mesh->SetCollisionObjectType(ECC_WorldDynamic);
		_Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		_Mesh->AttachToComponent(PawnMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, attachPoint);

		/*FString role = HasAuthority() ? "Server" : "Client";
		UE_LOG(LogTemp, Warning, TEXT("[%s] Before offset: %s"), *role, *_Mesh->GetRelativeTransform().ToString());*/

		SetEquipOffset(attachPoint);
	}
}

/////////////////////////////////////////////////////
void AGun::SetEquipOffset(FName Socket)
{
	const FTransform gripPointTransform = _MyOwner->GetMesh()->GetSocketTransform(Socket);
	const FTransform meshTransform = _Mesh->GetRelativeTransform();

	if(meshTransform.GetRotation().Rotator() == FRotator::ZeroRotator)
	{
		const FQuat offsetRot = gripPointTransform.InverseTransformRotation(_EquipOffset->GetComponentRotation().Quaternion()).Inverse();
		_Mesh->SetRelativeRotation(offsetRot);
	}

	if(meshTransform.GetLocation() == FVector::ZeroVector)
	{
		const FVector offsetLoc = -gripPointTransform.InverseTransformPosition(_EquipOffset->GetComponentLocation());
		_Mesh->SetRelativeLocation(offsetLoc);
	}

	/*FString role = HasAuthority() ? "Server" : "Client";
	UE_LOG(LogTemp, Warning, TEXT("[%s] After offset: %s"), *role, *_Mesh->GetRelativeTransform().ToString());*/
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
	USimpleInventory* inventory = Cast<USimpleInventory>(_MyOwner->GetComponentByClass(USimpleInventory::StaticClass()));
	ensure(inventory && HasAuthority());

	const bool bHasMag = inventory->GetMagazineFromInventory(_LoadedMagazine ? _LoadedMagazine->GetClass() : _GunStats.UsableMagazineClass);
	return bHasMag;
}

/////////////////////////////////////////////////////
bool AGun::OwnerHasMagazine() const
{
	USimpleInventory* inventory = Cast<USimpleInventory>(_MyOwner->GetComponentByClass(USimpleInventory::StaticClass()));	
	ensure(inventory);

	const TSubclassOf<AMagazine> magClass = _LoadedMagazine ? _LoadedMagazine->GetClass() : _GunStats.UsableMagazineClass;
	return inventory->HasMagazines(magClass);
}

/////////////////////////////////////////////////////
void AGun::OnAnimNotify_AttachMagToHand()
{
	if(_MyOwner && _LoadedMagazine)
	{
		_MyOwner->GrabItem(_LoadedMagazine, true);
		_LoadedMagazine = nullptr;
	}
}

void AGun::OnAnimNotify_DropMagazine()
{
	DropMagazine();	
}

void AGun::OnAnimNotify_CheckForAmmo()
{
	if (HasAuthority())
	{
		bool bHasMagazine = OwnerHasMagazine();
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
	
	_ItemToGrab = SpawnNewMagazine(_MyOwner->GetItemOffset(false));
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
	if (_CurrentGunState == EWeaponState::Reloading || OwnerHasMagazine() == false || (_LoadedMagazine && _LoadedMagazine->IsFull()))
	{
		return;
	}

	if (!HasAuthority())
	{
		_CurrentGunState = EWeaponState::Reloading;
	}

	Server_Reload();
}

/////////////////////////////////////////////////////
void AGun::OnItemGrab()
{
	if (_ItemToGrab)
	{
		_MyOwner->GrabItem(_ItemToGrab, true);
		_LoadedMagazine = nullptr;
	}
}

/////////////////////////////////////////////////////
void AGun::OnMagAttached()
{
	if (_MagToAttach)
	{
		AttachMagazine(_MagToAttach);
	}
}

/////////////////////////////////////////////////////
void AGun::Server_Reload_Implementation()
{
	if (_CurrentGunState == EWeaponState::Reloading || !OwnerHasMagazine())
	{
		return;
	}

	_CurrentGunState = EWeaponState::Reloading;
	
	UAnimInstance* animInstance = _MyOwner->GetMesh()->GetAnimInstance();
	if (_ReloadAnimation && animInstance)
	{
		Multicast_PlayReloadAnimation();		
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

		float reloadTime = 3.0f; // Default reloading time, if there is no reload animation for some reason.
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
	UAnimInstance* animInstance = _MyOwner->GetMesh()->GetAnimInstance();
	if (animInstance)
	{
		animInstance->Montage_Play(_ReloadAnimation, 1.f, EMontagePlayReturnType::MontageLength, 0.0f, false);
	}
}

/////////////////////////////////////////////////////
void AGun::Multicast_StopReloading_Implementation()
{
	if (_ReloadAnimation)
	{
		_MyOwner->StopAnimMontage(_ReloadAnimation);
	}
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
	IInteractable::Execute_SetCanBeInteractedWith(magazineInHand, false);
	if (HasAuthority())
	{
		magazineInHand->SetLifeSpan(30.0f);
	}
}

/////////////////////////////////////////////////////
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

	if (_MyOwner)
	{
		_MyOwner->OnReloadFinished.Broadcast(_MyOwner, this);
	}
}

/////////////////////////////////////////////////////
EFireMode AGun::ToggleFireMode()
{
	_CurrentFireModePointer = (_CurrentFireModePointer + 1) % _GunStats.FireModes.Num();
	_CurrentFireMode = _GunStats.FireModes[_CurrentFireModePointer];
	return _CurrentFireMode;
}

/////////////////////////////////////////////////////
UMeshComponent* AGun::GetMesh() const
{
	return _Mesh;
}

/////////////////////////////////////////////////////
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

	if(HasAuthority())
	{
		AHumanoid* humanoidOwner = Cast<AHumanoid>(GetInstigator());
		if (humanoidOwner)
		{
			Multicast_RepMyOwner(humanoidOwner);
		}
		
		FTransform spawnTransform = GetMesh()->GetSocketTransform("Magazine", RTS_World);
		_MagToAttach = SpawnNewMagazine(spawnTransform);
		AttachMagazine(_MagToAttach);
		
		OnDestroyed.AddDynamic(this, &AGun::HandleOnDestroyed);
	}
}

/////////////////////////////////////////////////////
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
	return _GunStats.Cooldown;
}

/////////////////////////////////////////////////////
AMagazine* AGun::GetMagazine() const
{
	return _LoadedMagazine;
}

/////////////////////////////////////////////////////
void AGun::SetFireMode(EFireMode NewFireMode)
{
	const bool bHasFireMode = _GunStats.FireModes.Find(NewFireMode);
	if (bHasFireMode)
	{
		_CurrentFireMode = NewFireMode;
	}
}

EFireMode AGun::GetCurrentFireMode() const
{
	return _CurrentFireMode;
}

/////////////////////////////////////////////////////
float AGun::GetRoundsPerMinute() const
{
	return 60.0f / GetCooldownTime();
}

/////////////////////////////////////////////////////
FTransform AGun::GetMuzzleTransform() const
{
	ensure(_Mesh);
	return _Mesh->GetSocketTransform(_MuzzleAttachPoint, RTS_World);
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
void AGun::HandleMuzzleFlash(bool bSpawnMuzzleFlash)
{
	if (bSpawnMuzzleFlash)
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
void AGun::PlayFireAnimation()
{
	if (_FireAnimation)
	{
		_Mesh->PlayAnimation(_FireAnimation, false);
	}
}

/////////////////////////////////////////////////////
void AGun::PlayFireSound()
{
	if (_FireSound)
	{
		UGameplayStatics::SpawnSoundAttached(_FireSound, _MyOwner->GetRootComponent());
	}
}

/////////////////////////////////////////////////////
void AGun::SpawnEjectedShell()
{
	if (_ShellEjectionPoint.IsValid() == false || _LoadedMagazine == nullptr)
	{
		return;
	}

	const AProjectile* projectile = _LoadedMagazine->GetProjectileClass().GetDefaultObject();
	if (projectile == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("No default projectile object found."));
		return;
	}

	const TSubclassOf<AActor> caseClass = projectile->GetProjectileCase();
	if (caseClass)
	{
		const FTransform spawnTransform = GetMesh()->GetSocketTransform(_ShellEjectionPoint);
		FActorSpawnParameters spawnParams = FActorSpawnParameters();
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* cartridgeCase = GetWorld()->SpawnActor<AActor>(caseClass, spawnTransform, spawnParams);
		if (cartridgeCase == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("No cartridge case spawned on %s!"), *GetName());
			return;
		}

		UMeshComponent* caseMesh = Cast<UMeshComponent>(cartridgeCase->GetComponentByClass(UMeshComponent::StaticClass()));
		if (caseMesh)
		{
			FTransform transform = cartridgeCase->GetActorTransform();			
			FVector rightVector = transform.TransformVector(FVector(0.0f, 1.0f, 0.0f));
			FVector velocity = GetOwner()->GetVelocity();

			FVector impulse = (rightVector * 500.0f) + velocity;
			caseMesh->AddImpulse(impulse, NAME_None, true);
		}
	}
}


/////////////////////////////////////////////////////
					/* Networking */
/////////////////////////////////////////////////////
void AGun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGun, _ItemToGrab);
	DOREPLIFETIME(AGun, _MagToAttach);
}

/////////////////////////////////////////////////////
void AGun::HandleOnFire_Multicast_Implementation()
{
	_MyOwner->OnWeaponFire.Broadcast(_MyOwner, this);

	SpawnEjectedShell();
	PlayFireSound();
	PlayFireAnimation();
	HandleMuzzleFlash(true);
}

/////////////////////////////////////////////////////
void AGun::Server_OnStopFire_Implementation()
{
	_CurrentGunState = EWeaponState::Idle;
	_SalvoCount = 0;
	HandleMuzzleFlash_Multicast(false);
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
void AGun::OnFire_Server_Implementation(EFireMode FireMode, FTransform SpawnTransform)
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
		HandleOnFire_Multicast();
	}
	else
	{
		Server_OnStopFire();
	}	
}

bool AGun::OnFire_Server_Validate(EFireMode FireMode, FTransform SpawnTransform)
{
	return true;
}

/////////////////////////////////////////////////////
void AGun::HandleMuzzleFlash_Multicast_Implementation(bool bSpawnMuzzleFlash)
{
	HandleMuzzleFlash(bSpawnMuzzleFlash);
}