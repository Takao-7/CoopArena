// Fill out your copyright notice in the Description page of Project Settings.

#include "Humanoid.h"
#include "Engine/World.h"
#include "Interactable.h"
#include "TimerManager.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/HealthComponent.h"
#include "Components/BasicAnimationSystemComponent.h"
#include "Components/InventoryComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapons/Gun.h"
#include "GameFramework/Controller.h"
#include "UnrealNetwork.h"


// Sets default values
AHumanoid::AHumanoid()
{
	PrimaryActorTick.bCanEverTick = true;

	_BaseTurnRate = 45.f;
	_BaseLookUpRate = 45.f;

	_DefaultInpulsOnDeath = 500.0f;

	_EquippedWeaponAttachPoint = "GripPoint";

	_DroppedItemSpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("Dropped item spawn point"));
	_DroppedItemSpawnPoint->SetupAttachment(RootComponent);

	bIsSprinting = false;
	bIsProne = false;
	bIsCrouched = false;
	bIsAiming = false;

	_MaxSprintSpeed = 650.0f;
	bToggleProne = true;

	SetReplicates(true);
	SetReplicateMovement(true);

	GetMesh()->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	BASComponent = CreateDefaultSubobject<UBasicAnimationSystemComponent>(TEXT("Basic Animation System"));
	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	AddOwnedComponent(HealthComponent);
	AddOwnedComponent(BASComponent);
	AddOwnedComponent(Inventory);
}

/////////////////////////////////////////////////////
AGun* AHumanoid::GetEquippedGun() const
{
	return _EquippedWeapon;
}

void AHumanoid::SetEquippedWeapon(AGun* Weapon)
{
	_EquippedWeapon = Weapon;
}

/////////////////////////////////////////////////////
bool AHumanoid::Set_ComponentIsBlockingFiring(bool bIsBlocking, UActorComponent* Component)
{
	if (Component == nullptr || (_BlockingComponent && _BlockingComponent != Component))
	{
		return false;
	}

	bComponentBlocksFiring = bIsBlocking;

	if (bIsBlocking)
	{
		_BlockingComponent = Component;
	}
	else
	{
		_BlockingComponent = nullptr;
	}

	return true;
}


/////////////////////////////////////////////////////
bool AHumanoid::IsAiming_Implementation()
{
	return bIsAiming;
}

/////////////////////////////////////////////////////
EWEaponType AHumanoid::GetEquippedWeaponType_Implementation()
{	
	if (_EquippedWeapon)
	{
		return _EquippedWeapon->GetWeaponType();
	}
	else
	{
		return EWEaponType::None;
	}
}

/////////////////////////////////////////////////////
EMovementType AHumanoid::GetMovementType_Implementation()
{
	if (FMath::IsNearlyZero(GetVelocity().Size(), 0.1f))
	{
		return EMovementType::Idle;
	}
	else if (bIsSprinting)
	{
		return EMovementType::Sprinting;
	}
	else
	{
		return EMovementType::Moving;
	}
}

/////////////////////////////////////////////////////
EMovementAdditive AHumanoid::GetMovementAdditive_Implementation()
{
	if (bIsCrouched)
	{
		return EMovementAdditive::Crouch;
	}
	else if (bIsProne)
	{
		return EMovementAdditive::Prone;
	}
	else
	{
		return EMovementAdditive::None;
	}
}

/////////////////////////////////////////////////////
void AHumanoid::OnEquipWeapon()
{
	HolsterWeapon_Event.Broadcast(_EquippedWeapon);	
}

/////////////////////////////////////////////////////
void AHumanoid::BeginPlay()
{
	Super::BeginPlay();

	_MaxWalkingSpeed = GetCharacterMovement()->MaxWalkSpeed;
	if (HasAuthority())
	{
		SetUpDefaultEquipment();
	}
}

/////////////////////////////////////////////////////
void AHumanoid::SetSprinting(bool bWantsToSprint)
{
	bool bCanSprint = BASComponent->GetActorVariables().bIsMovingForward;
	if (bWantsToSprint && bCanSprint)
	{
		if (bIsAiming)
		{
			ToggleAiming();
		}
		GetCharacterMovement()->MaxWalkSpeed = _MaxSprintSpeed;
		bIsSprinting = true;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = _MaxWalkingSpeed;
		bIsSprinting = false;
	}
	Server_SetSprinting(bIsSprinting);
}

/////////////////////////////////////////////////////
void AHumanoid::FireEquippedWeapon()
{
	if (CanFire() && _EquippedWeapon)
	{
		_EquippedWeapon->OnFire();
	}
}

void AHumanoid::StopFireEquippedWeapon()
{
	if (_EquippedWeapon)
	{
		_EquippedWeapon->OnStopFire();
	}
}

/////////////////////////////////////////////////////
void AHumanoid::MoveForward(float Value)
{
	if (Controller && Value != 0.0f)
	{
		FRotator Rotation = Controller->GetControlRotation();
		Rotation.Pitch = 0.0f;
		const FVector Direction = Rotation.Vector();
		AddMovementInput(Direction, Value);
	}
	else if (Value < 0.0f && bIsSprinting)
	{
		SetSprinting(false);
	}
}

void AHumanoid::MoveRight(float Value)
{
	if (Controller && Value != 0.0f && !bIsSprinting)
	{
		FRotator Rotation = Controller->GetControlRotation();
		Rotation.Pitch = 0.0f;
		const FVector Direction = Rotation.Vector().RotateAngleAxis(90.0f, FVector(0.0f, 0.0f, 1.0f));
		AddMovementInput(Direction, Value);
	}
}

/////////////////////////////////////////////////////
void AHumanoid::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * _BaseTurnRate * GetWorld()->GetDeltaSeconds());	
}

void AHumanoid::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * _BaseTurnRate * GetWorld()->GetDeltaSeconds());	
}

/////////////////////////////////////////////////////
void AHumanoid::SetProne(bool bProne)
{
	bIsProne = bProne;
}

/////////////////////////////////////////////////////
void AHumanoid::ToggleAiming()
{
	if (!bIsSprinting)
	{
		bIsAiming = !bIsAiming;
	}
	else
	{
		bIsAiming = false;
	}
}

/////////////////////////////////////////////////////
void AHumanoid::SetCrouch(bool bSprint)
{
	if (bSprint)
	{
		Crouch();
	}
	else
	{
		UnCrouch();
	}
}

/////////////////////////////////////////////////////
void AHumanoid::ToggleJump()
{
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		float veloctiy_abs = FMath::Abs(GetVelocity().Size());
		if (veloctiy_abs <= 10.0f)
		{
			FTimerHandle jumpTH;
			GetWorld()->GetTimerManager().SetTimer(jumpTH, this, &ACharacter::Jump, 0.275f);
		}
		else
		{
			Jump();
		}
	}
	else
	{
		StopJumping();
	}
}

/////////////////////////////////////////////////////
void AHumanoid::ReloadWeapon()
{
	if (_EquippedWeapon)
	{
		_EquippedWeapon->Reload();
	}
}

/////////////////////////////////////////////////////
void AHumanoid::ChangeWeaponFireMode()
{
	if (_EquippedWeapon)
	{
		_EquippedWeapon->ToggleFireMode();
	}
}

/////////////////////////////////////////////////////
void AHumanoid::GrabItem(AItemBase* ItemToGrab, bool bKeepRelativeOffset)
{
	_ItemInHand = ItemToGrab;
	CalcAndSafeActorOffset(ItemToGrab);
	FName handSocket = "HandLeft";
	ItemToGrab->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, handSocket);
	
	if (bKeepRelativeOffset)
	{
		ItemToGrab->AddActorLocalTransform(_ItemOffset);		
	}
}

/////////////////////////////////////////////////////
FTransform AHumanoid::CalcAndSafeActorOffset(AActor* OtherActor)
{
	FTransform offset;
	FTransform handTransform = GetMesh()->GetSocketTransform("HandLeft");

	FVector itemLocation = OtherActor->GetActorLocation();
	offset.SetLocation(handTransform.InverseTransformPosition(itemLocation));

	FRotator itemRotation = OtherActor->GetActorRotation();
	offset.SetRotation(handTransform.InverseTransformRotation(itemRotation.Quaternion()));
	
	_ItemOffset = offset;
	return offset;
}

/////////////////////////////////////////////////////
AItemBase* AHumanoid::DropItem()
{
	if (_ItemInHand == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s tried to drop an item without having any to drop!"), *GetName());
		return nullptr;
	}
	AItemBase* itemToDrop = _ItemInHand;	
	itemToDrop->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	itemToDrop->ShouldSimulatePhysics(true);
	
	_ItemInHand = nullptr;
	return itemToDrop;
}

/////////////////////////////////////////////////////
bool AHumanoid::CanFire() const
{
	return !bIsSprinting && GetCharacterMovement()->IsMovingOnGround() && !bComponentBlocksFiring;
}

/////////////////////////////////////////////////////
FName AHumanoid::GetEquippedWeaponAttachPoint() const
{
	return _EquippedWeaponAttachPoint;
}

/////////////////////////////////////////////////////
void AHumanoid::SetUpDefaultEquipment()
{
	if (_DefaultGun == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("DefaultWeapon is null."));
		return;
	}
	if (!GetEquippedWeaponAttachPoint().IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponAttachPoint is not valid!"));
		return;
	}

	if(HasAuthority())
	{
		_WeaponToEquip = SpawnWeapon(_DefaultGun);
		OnWeaponEquip();
	}
}

/////////////////////////////////////////////////////
AGun* AHumanoid::SpawnWeapon(TSubclassOf<AGun> Class)
{
	FTransform spawnTransform = FTransform();
	GetWeaponSpawnTransform(spawnTransform);

	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	params.Owner = this;
	params.Instigator = this;

	return GetWorld()->SpawnActor<AGun>(Class, spawnTransform, params);
}

/////////////////////////////////////////////////////
void AHumanoid::GetWeaponSpawnTransform(FTransform& OutTransform)
{
	FVector location;
	FRotator rotation;
	FName AttachPoint = GetEquippedWeaponAttachPoint();
	GetMesh()->GetSocketWorldLocationAndRotation(AttachPoint, location, rotation);

	OutTransform.SetLocation(location);
	OutTransform.SetRotation(rotation.Quaternion());
}

/////////////////////////////////////////////////////
FTransform AHumanoid::GetItemOffset(bool bInLocalSpace /*= true*/)
{
	FTransform offset = FTransform(_ItemOffset);
	if (!bInLocalSpace)
	{
		const FTransform handTransform = GetMesh()->GetSocketTransform("HandLeft");
		offset.SetLocation(handTransform.TransformPosition(offset.GetLocation()));
		offset.SetRotation(handTransform.TransformRotation(offset.GetRotation()));
	}
	return offset;
}

/////////////////////////////////////////////////////
void AHumanoid::Multicast_ClearItemInHand_Implementation()
{
	if (_ItemInHand && !_ItemInHand->IsAttachedTo(this))
	{
		_ItemInHand = nullptr;
	}
}


/////////////////////////////////////////////////////
					/* Networking */
/////////////////////////////////////////////////////
void AHumanoid::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHumanoid, bIsSprinting);
	DOREPLIFETIME(AHumanoid, bIsAiming);
	DOREPLIFETIME(AHumanoid, bIsProne);
	DOREPLIFETIME(AHumanoid, _WeaponToEquip);
}

/////////////////////////////////////////////////////
void AHumanoid::Server_SetSprinting_Implementation(bool bSprint)
{
	bIsSprinting = bSprint;
	if (bIsSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = _MaxSprintSpeed;
		if (bIsAiming)
		{
			ToggleAiming();
		}
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = _MaxWalkingSpeed;
	}
}

bool AHumanoid::Server_SetSprinting_Validate(bool bSprint)
{
	return true;
}

/////////////////////////////////////////////////////
void AHumanoid::SetWeaponToEquip(AGun* Weapon)
{
	if (HasAuthority())
	{
		_WeaponToEquip = Weapon;
		OnWeaponEquip();
	}
}

/////////////////////////////////////////////////////
void AHumanoid::OnWeaponEquip()
{
	if(_WeaponToEquip)
	{
		SetEquippedWeapon(_WeaponToEquip);
		_EquippedWeapon->OnEquip(this);
	}
}

/////////////////////////////////////////////////////
void AHumanoid::OnRep_bIsSprining()
{
	GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? _MaxSprintSpeed : _MaxWalkingSpeed;
}

/////////////////////////////////////////////////////
void AHumanoid::RepMaxWalkSpeed_Implementation(float NewMaxWalkSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewMaxWalkSpeed;
}

bool AHumanoid::RepMaxWalkSpeed_Validate(float NewMaxWalkSpeed)
{
	return true;
}
