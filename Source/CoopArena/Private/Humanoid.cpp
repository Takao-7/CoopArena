// Fill out your copyright notice in the Description page of Project Settings.

#include "Humanoid.h"
#include "Engine/World.h"
#include "Interactable.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/ArrowComponent.h"
#include "Weapons/Gun.h"


// Sets default values
AHumanoid::AHumanoid()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	// set our turn rates for input
	_BaseTurnRate = 45.f;
	_BaseLookUpRate = 45.f;

	bAlreadyDied = false;
	_WeaponAttachPoint = "GripPoint";

	_DroppedItemSpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("Dropped item spawn point"));
	_DroppedItemSpawnPoint->SetupAttachment(RootComponent);

	bIsAiming = false;
}


AGun* AHumanoid::GetEquippedGun() const
{
	return _EquippedWeapon;
}


void AHumanoid::SetEquippedWeapon(AGun* weapon)
{
	_EquippedWeapon = weapon;
}


// Called when the game starts or when spawned
void AHumanoid::BeginPlay()
{
	Super::BeginPlay();
	
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
	if (value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), value);
	}
}


void AHumanoid::MoveRight(float value)
{
	if (value != 0.0f && !bIsSprinting)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), value);
	}
}


void AHumanoid::TurnAtRate(float rate)
{
	AddControllerYawInput(rate * _BaseTurnRate * GetWorld()->GetDeltaSeconds());
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
		GetCharacterMovement()->MaxWalkSpeed /= 2;
		_BaseTurnRate *= 2;
		bIsSprinting = false;
	}
	else if (GetVelocity().X >= 10.0f)
	{
		GetCharacterMovement()->MaxWalkSpeed *= 2;
		_BaseTurnRate /= 2;
		bIsSprinting = true;
	}
}


/////////////////////////////////////////////////////
bool AHumanoid::IsAlive() const
{
	return true;
}

/////////////////////////////////////////////////////
bool AHumanoid::CanFire() const
{
	return IsAlive() && !bIsSprinting;
}


FName AHumanoid::GetWeaponAttachPoint() const
{
	return _WeaponAttachPoint;
}


void AHumanoid::SetUpDefaultEquipment()
{
	if (_DefaultGun == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("DefaultWeapon is null!"));
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
}


// Called every frame
void AHumanoid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}