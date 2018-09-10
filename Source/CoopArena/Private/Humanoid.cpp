// Fill out your copyright notice in the Description page of Project Settings.

#include "Humanoid.h"
#include "Engine/World.h"
#include "Interactable.h"
#include "TimerManager.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SkeletalMeshComponent.h"
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

<<<<<<< HEAD
	_MaxSprintSpeed = 650.0f;
	bToggleProne = true;
}
=======
	bIsSprinting = false;
	bIsProne = false;
	bIsCrouched = false;
	bIsAiming = false;

	_MaxSprintSpeed = 650.0f;
	bToggleProne = true;
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8

	SetReplicates(true);
	SetReplicateMovement(true);

	GetMesh()->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones;
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


bool AHumanoid::SetComponentIsBlockingFiring(bool bIsBlocking, UActorComponent* Component)
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
	if (GetLastMovementInputVector().Size() == 0.0f)
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

<<<<<<< HEAD

=======
/////////////////////////////////////////////////////
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
void AHumanoid::BeginPlay()
{
	Super::BeginPlay();

<<<<<<< HEAD
	_MaxWalkingSpeed = GetCharacterMovement()->GetMaxSpeed();	
	SetUpDefaultEquipment();
=======
	_MaxWalkingSpeed = GetCharacterMovement()->MaxWalkSpeed;
	if (HasAuthority())
	{
		SetUpDefaultEquipment();
	}
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
}

/////////////////////////////////////////////////////
void AHumanoid::SetSprinting(bool bSprint)
{
	if (bSprint)
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
	Server_SetSprinting(bSprint);
}

void AHumanoid::SetSprinting(bool bSprint)
{
	if (bSprint)
	{
		GetCharacterMovement()->MaxWalkSpeed = _MaxSprintSpeed;
		bIsSprinting = true;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = _MaxWalkingSpeed;
		bIsSprinting = false;
	}
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

<<<<<<< HEAD

/////////////////////////////////////////////////////
void AHumanoid::MoveForward(float Value)
{
	FRotator controlRotation = GetControlRotation();
	controlRotation.Pitch = 0.0f;
	FVector direction = controlRotation.Vector();
	direction.Normalize();

	AddMovementInput(direction, Value);
	if (Value > 0.0f)
	{
		bIsMovingForward = true;
	}
	else
	{
		bIsMovingForward = false;
=======
/////////////////////////////////////////////////////
void AHumanoid::MoveForward(float Value)
{
	if (Controller && Value != 0.0f)
	{
		// Limit pitch when walking or falling
		const bool bLimitRotation = (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling());
		const FRotator Rotation = bLimitRotation ? GetActorRotation() : Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
	else if (Value < 0.0f && bIsSprinting)
	{
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
		SetSprinting(false);
	}
}

void AHumanoid::MoveRight(float Value)
{
	if (Value != 0.0f && !bIsSprinting)
	{
		const FQuat Rotation = GetActorQuat();
		const FVector Direction = FQuatRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
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

<<<<<<< HEAD

=======
/////////////////////////////////////////////////////
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
void AHumanoid::SetProne(bool bProne)
{
	bIsProne = bProne;
}

/////////////////////////////////////////////////////
void AHumanoid::ToggleAiming()
{
<<<<<<< HEAD
	bIsAiming = !bIsAiming;
}


=======
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
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
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

<<<<<<< HEAD

void AHumanoid::GrabItem(AItemBase* ItemToGrab, bool bKeepRelativeOffset, FTransform Offset)
{
	EAttachmentRule attachmentRule;
	bKeepRelativeOffset ? attachmentRule = EAttachmentRule::KeepWorld : attachmentRule = EAttachmentRule::SnapToTarget;
	FAttachmentTransformRules rules = FAttachmentTransformRules(attachmentRule, false);
	FName handSocket = "HandLeft";

	ItemToGrab->AttachToComponent(GetMesh(), rules, handSocket);
	_ItemInHand = ItemToGrab;
	if (bKeepRelativeOffset)
	{
		CalcAndSafeActorOffset(ItemToGrab);
	}
	else
	{
		ItemToGrab->SetActorRelativeTransform(Offset);
	}	
}


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


AItemBase* AHumanoid::DropItem()
{
	if (_ItemInHand == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s tried to drop an item without having any to drop!"), *GetName());
		return nullptr;
	}
	AItemBase* itemToDrop = _ItemInHand;	
	itemToDrop->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	itemToDrop->SetSimulatePhysics(true);
	
	_ItemInHand = nullptr;
	return itemToDrop;
=======
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
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
}


/////////////////////////////////////////////////////
FTransform AHumanoid::CalcAndSafeActorOffset(AActor* OtherActor)
{
<<<<<<< HEAD
	return !bIsSprinting && GetCharacterMovement()->IsMovingOnGround() && !bComponentBlocksFiring;
=======
	FTransform offset;
	FTransform handTransform = GetMesh()->GetSocketTransform("HandLeft");

	FVector itemLocation = OtherActor->GetActorLocation();
	offset.SetLocation(handTransform.InverseTransformPosition(itemLocation));

	FRotator itemRotation = OtherActor->GetActorRotation();
	offset.SetRotation(handTransform.InverseTransformRotation(itemRotation.Quaternion()));
	
	_ItemOffset = offset;
	return offset;
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
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

<<<<<<< HEAD
FName AHumanoid::GetEquippedWeaponAttachPoint() const
{
	return _EquippedWeaponAttachPoint;
=======
/////////////////////////////////////////////////////
bool AHumanoid::CanFire() const
{
	return !bIsSprinting && GetCharacterMovement()->IsMovingOnGround() && !bComponentBlocksFiring;
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
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

<<<<<<< HEAD
// Called every frame
void AHumanoid::Tick(float DeltaTime)
=======
	DOREPLIFETIME(AHumanoid, bIsSprinting);
	DOREPLIFETIME(AHumanoid, bIsAiming);
	DOREPLIFETIME(AHumanoid, bIsProne);
	DOREPLIFETIME(AHumanoid, _WeaponToEquip);
}

/////////////////////////////////////////////////////
void AHumanoid::Server_SetSprinting_Implementation(bool bSprint)
{
	if (bSprint)
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
}

bool AHumanoid::Server_SetSprinting_Validate(bool bSprint)
{
	return true;
}

/////////////////////////////////////////////////////
void AHumanoid::SetWeaponToEquip(AGun* Weapon)
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
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
<<<<<<< HEAD
		SetSprinting(false);
=======
		SetEquippedWeapon(_WeaponToEquip);
		_EquippedWeapon->OnEquip(this);
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
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
