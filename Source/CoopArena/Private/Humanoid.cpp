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
#include "UnrealNetwork.h"


// Sets default values
AHumanoid::AHumanoid()
{
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
}

AGun* AHumanoid::GetEquippedGun() const
{
	return _EquippedWeapon;
}

void AHumanoid::SetEquippedWeapon(AGun* Weapon)
{
	_EquippedWeapon = Weapon;
}

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

void AHumanoid::BeginPlay()
{
	Super::BeginPlay();

	_MaxWalkingSpeed = GetCharacterMovement()->MaxWalkSpeed;
	if (HasAuthority())
	{
		SetUpDefaultEquipment();
	}
}

void AHumanoid::SetSprinting(bool bSprint)
{
	Server_SetSprinting(bSprint);
}


/////////////////////////////////////////////////////
void AHumanoid::FireEquippedWeapon()
{
	if (CanFire() && _EquippedWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Humanoid name: %s. Weapon owner: %s"), *GetName(), *_EquippedWeapon->GetOwner()->GetName());
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
	if (Controller == nullptr || Value == 0.0f)
	{
		return;
	}
	FRotator controlRotation = GetControlRotation();
	controlRotation.Pitch = 0.0f;
	controlRotation.Roll = 0.0f;
	
	FVector direction(1.0f, 0.0f, 0.0f);
	direction = controlRotation.RotateVector(direction);

	AddMovementInput(direction, Value);
	if (Value < 0.0f && bIsSprinting)
	{
		SetSprinting(false);
	}
}

void AHumanoid::MoveRight(float Value)
{
	if (bIsSprinting || Controller == nullptr || Value == 0.0f)
	{
		return;
	}
	
	FRotator controlRotation = GetControlRotation();
	controlRotation.Pitch = 0.0f;
	controlRotation.Roll = 0.0f;

	FVector direction(0.0f, 1.0f, 0.0f);
	direction = controlRotation.RotateVector(direction);
		
	AddMovementInput(direction, Value);
}

void AHumanoid::TurnAtRate(float Rate)
{
	if (Controller)
	{
		AddControllerYawInput(Rate * _BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

void AHumanoid::LookUpAtRate(float Rate)
{
	if (Controller)
	{
		AddControllerPitchInput(Rate * _BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

void AHumanoid::SetProne(bool bProne)
{
	bIsProne = bProne;
}

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

void AHumanoid::ReloadWeapon()
{
	if (_EquippedWeapon)
	{
		_EquippedWeapon->Reload();
	}
}

void AHumanoid::ChangeWeaponFireMode()
{
	if (_EquippedWeapon)
	{
		_EquippedWeapon->ToggleFireMode();
	}
}

void AHumanoid::GrabItem(AItemBase* ItemToGrab, bool bKeepRelativeOffset, FTransform Offset)
{
	/*if (!HasAuthority())
	{
		return;
	}*/
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
	
	_EquippedWeapon = AGun::SpawnGunAttached(this, _DefaultGun);
}


/////////////////////////////////////////////////////
					/* Networking */
/////////////////////////////////////////////////////
void AHumanoid::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHumanoid, bIsSprinting);
	DOREPLIFETIME(AHumanoid, bIsAiming);
	DOREPLIFETIME(AHumanoid, _EquippedWeapon);
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
		//UE_LOG(LogTemp, Warning, TEXT("MaxSprintSpeed: %f"), _MaxSprintSpeed);
		GetCharacterMovement()->MaxWalkSpeed = _MaxSprintSpeed;
		bIsSprinting = true;
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("MaxWalkSpeed: %f"), _MaxWalkingSpeed);
		GetCharacterMovement()->MaxWalkSpeed = _MaxWalkingSpeed;
		bIsSprinting = false;
	}
}

bool AHumanoid::Server_SetSprinting_Validate(bool bSprint)
{
	return true;
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
