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


// Sets default values
AHumanoid::AHumanoid()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// set our turn rates for input
	_BaseTurnRate = 45.f;
	_BaseLookUpRate = 45.f;

	_DefaultInpulsOnDeath = 500.0f;

	_WeaponAttachPoint = "GripPoint";

	_DroppedItemSpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("Dropped item spawn point"));
	_DroppedItemSpawnPoint->SetupAttachment(RootComponent);

	bIsAiming = false;
	bIsMovingForward = false;
	bAlreadyDied = false;
	bIsSprinting = false;
	bIsCrouched = false;
	bIsProne = false;

	_SprintSpeedIncrease = 2.0f;
}


AGun* AHumanoid::GetEquippedGun() const
{
	return _EquippedWeapon;
}


void AHumanoid::SetEquippedWeapon(AGun* Weapon)
{
	_EquippedWeapon = Weapon;
}


/////////////////////////////////////////////////////
bool AHumanoid::IsAiming_Implementation()
{
	return bIsAiming;
}


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


void AHumanoid::ToggleProne()
{
	bIsProne = !bIsProne;
}


/////////////////////////////////////////////////////
void AHumanoid::OnEquipWeapon()
{
	EquipWeapon_Event.Broadcast();
}


// Called when the game starts or when spawned
void AHumanoid::BeginPlay()
{
	Super::BeginPlay();

	_DefaultMaxWalkingSpeed = GetCharacterMovement()->GetMaxSpeed();	
	SetUpDefaultEquipment();
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
void AHumanoid::OnDeath()
{
	if (bAlreadyDied)
	{
		return;
	}
	bAlreadyDied = true;

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);


	FTimerDelegate delegate;
	delegate.BindLambda([this]
	{
		if (_EquippedWeapon)
		{
			_EquippedWeapon->OnUnequip(true);
		}
	});

	FTimerHandle handle;
	GetWorld()->GetTimerManager().SetTimer(handle, delegate, 1.0f, false);
}


/////////////////////////////////////////////////////
void AHumanoid::DeactivateCollisionCapsuleComponent()
{
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
}


/////////////////////////////////////////////////////
void AHumanoid::MoveForward(float Value)
{
	FRotator controlRotation = GetControlRotation();
	controlRotation.Pitch = 0.0f;
	FVector direction = controlRotation.Vector();
	direction.Normalize();

	if (Value > 0.0f)
	{

		AddMovementInput(direction, Value);
		bIsMovingForward = true;
	}
	else if (Value < 0.0f)
	{
		AddMovementInput(direction, Value);
		bIsMovingForward = false;
		SetSprinting(false);
	}
	else
	{
		bIsMovingForward = false;
		SetSprinting(false);
	}
}


void AHumanoid::MoveRight(float Value)
{
	if (Value != 0.0f && !bIsSprinting)
	{
		FRotator controlRotation = GetControlRotation();
		controlRotation.Pitch = 0.0f;

		FVector direction = controlRotation.Vector();
		direction = direction.RotateAngleAxis(90.0f, FVector(0.f, 0.f, 1.0f));
		direction.Normalize();
		
		AddMovementInput(direction, Value);
	}
}


void AHumanoid::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * _BaseTurnRate * GetWorld()->GetDeltaSeconds());
}


void AHumanoid::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * _BaseLookUpRate * GetWorld()->GetDeltaSeconds());	
}


void AHumanoid::ToggleCrouch()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}


void AHumanoid::ToggleAiming()
{
	if (bIsAiming)
	{
		bIsAiming = false;
	}
	else
	{
		bIsAiming = true;
	}
}


void AHumanoid::StopSprinting()
{
	SetSprinting(false);
}


void AHumanoid::StartSprinting()
{
	SetSprinting(true);
}


void AHumanoid::SetSprinting(bool bSprint)
{
	if (bSprint)
	{
		GetCharacterMovement()->MaxWalkSpeed = _DefaultMaxWalkingSpeed * _SprintSpeedIncrease;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = _DefaultMaxWalkingSpeed;
	}	
	bIsSprinting = bSprint;
}


void AHumanoid::ToggleJump()
{
	if (!bIsJumping)
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
		bIsJumping = true;
	}
	else
	{
		StopJumping();
		bIsJumping = false;
	}
}


void AHumanoid::ReloadWeapon()
{
	if (_EquippedWeapon)
	{
		_EquippedWeapon->ReloadWeapon();
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
}


/////////////////////////////////////////////////////
bool AHumanoid::IsAlive() const
{
	return !bAlreadyDied;
}

/////////////////////////////////////////////////////
bool AHumanoid::CanFire() const
{
	return IsAlive() && !bIsSprinting && !bIsJumping;
}


FName AHumanoid::GetWeaponAttachPoint() const
{
	return _WeaponAttachPoint;
}


void AHumanoid::SetUpDefaultEquipment()
{
	if (_DefaultGun == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("DefaultWeapon is null."));
		return;
	}
	if (!GetWeaponAttachPoint().IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponAttachPoint is not valid!"));
		return;
	}

	_EquippedWeapon = GetWorld()->SpawnActor<AGun>(_DefaultGun);
	_EquippedWeapon->OnEquip(this);
}


void AHumanoid::Kill()
{
	OnDeath();
}


void AHumanoid::Kill(const UDamageType* DamageType, FVector Direction, FVector HitLocation)
{
	OnDeath();

	/*FTimerDelegate delegate;
	delegate.BindUFunction(this, "ImpulseOnDeath", Direction, HitLocation);
	FTimerHandle timerHandle;
	GetWorld()->GetTimerManager().SetTimer(timerHandle, delegate, 0.1f, false);*/
}


void AHumanoid::ImpulseOnDeath(FVector Direction, FVector HitLocation)
{
	GetMesh()->AddImpulseAtLocation(Direction * _DefaultInpulsOnDeath, HitLocation, "Head");
}


// Called every frame
void AHumanoid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsSprinting && bIsAiming)
	{
		SetSprinting(false);	
	}
}