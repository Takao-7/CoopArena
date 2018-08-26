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

	_WeaponAttachPoint = "GripPoint";

	_DroppedItemSpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("Dropped item spawn point"));
	_DroppedItemSpawnPoint->SetupAttachment(RootComponent);

	bIsAiming = false;
	bIsMovingForward = false;
	bAlreadyDied = false;
	bIsSprinting = false;
	bIsCrouched = false;
	bUseControllerRotationYaw = false;

	_SprintSpeedIncrease = 2.0f;
}


AGun* AHumanoid::GetEquippedGun() const
{
	return _EquippedWeapon;
}


void AHumanoid::SetEquippedWeapon(AGun* weapon)
{
	_EquippedWeapon = weapon;
}


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
void AHumanoid::OnDeath(const FHitResult& hitInfo, FVector direction)
{

}


/////////////////////////////////////////////////////
void AHumanoid::DeactivateCollisionCapsuleComponent()
{
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->bGenerateOverlapEvents = false;
}


/////////////////////////////////////////////////////
void AHumanoid::MoveForward(float value)
{
	if (value > 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), value);
		bIsMovingForward = true;
	}
	else if (value < 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), value);
		bIsMovingForward = false;
		SetSprinting(false);
	}
	else
	{
		bIsMovingForward = false;
		SetSprinting(false);
	}

	_FeetDirection = GetControlRotation().Yaw;
}


void AHumanoid::MoveRight(float value)
{
	if (value != 0.0f && !bIsSprinting)
	{
		AddMovementInput(GetActorRightVector(), value);
		_FeetDirection = GetControlRotation().Yaw;
	}
}


void AHumanoid::TurnAtRate(float rate)
{
	AddControllerYawInput(rate * _BaseTurnRate * GetWorld()->GetDeltaSeconds());

	float Yaw = GetControlRotation().Yaw;
	if (Yaw - _FeetDirection >= 85.0f)
	{
		_FeetDirection = Yaw;
	}
}


void AHumanoid::LookUpAtRate(float rate)
{
	AddControllerPitchInput(rate * _BaseLookUpRate * GetWorld()->GetDeltaSeconds());	
}


void AHumanoid::ToggleCrouch()
{
	if (this->bIsCrouched)
	{
		this->UnCrouch();
	}
	else
	{
		this->Crouch();
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


void AHumanoid::ToggleSprinting()
{
	if (bIsSprinting)
	{
		SetSprinting(false);
	}
	else if (bIsMovingForward)
	{
		SetSprinting(true);
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
	if (bAlreadyDied)
	{
		return;
	}
	bAlreadyDied = true;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);
}


// Called every frame
void AHumanoid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsSprinting)
	{
		if (bIsAiming)
		{
			SetSprinting(false);
		}
	}
}