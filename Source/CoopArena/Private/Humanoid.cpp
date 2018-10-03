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
#include "Components/RespawnComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapons/Gun.h"
#include "GameFramework/Controller.h"
#include "UnrealNetwork.h"


// Sets default values
AHumanoid::AHumanoid()
{
	PrimaryActorTick.bCanEverTick = true;

	m_BaseTurnRate = 45.f;
	m_BaseLookUpRate = 45.f;

	m_EquippedWeaponAttachPoint = "GripPoint";

	m_DroppedItemSpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("Dropped item spawn point"));
	m_DroppedItemSpawnPoint->SetupAttachment(RootComponent);

	m_bIsSprinting = false;
	m_bIsProne = false;
	bIsCrouched = false;
	m_bIsAiming = false;

	m_MaxForwardSpeed = 600.0f;
	m_MaxCrouchingSpeed = 200.0f;
	m_MaxBackwardsSpeed = 300.0f;
	m_SprintingSpeedThreshold = 355.0f;

	m_bToggleProne = true;
	m_bToggleCrouching = true;

	SetReplicates(true);
	SetReplicateMovement(true);

	GetMesh()->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	BASComponent = CreateDefaultSubobject<UBasicAnimationSystemComponent>(TEXT("Basic Animation System"));
	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	RespawnComponent = CreateDefaultSubobject<URespawnComponent>(TEXT("Respawn"));
	AddOwnedComponent(HealthComponent);
	AddOwnedComponent(BASComponent);
	AddOwnedComponent(Inventory);
	AddOwnedComponent(RespawnComponent);
}

/////////////////////////////////////////////////////
AGun* AHumanoid::GetEquippedGun() const
{
	return m_EquippedWeapon;
}

void AHumanoid::SetEquippedWeapon(AGun* Weapon)
{
	m_EquippedWeapon = Weapon;
}

/////////////////////////////////////////////////////
bool AHumanoid::SetComponentIsBlockingFiring(bool bIsBlocking, UActorComponent* Component)
{
	if (Component == nullptr || (m_BlockingComponent && m_BlockingComponent != Component))
	{
		return false;
	}

	m_bComponentBlocksFiring = bIsBlocking;

	if (bIsBlocking)
	{
		m_BlockingComponent = Component;
	}
	else
	{
		m_BlockingComponent = nullptr;
	}

	return true;
}

/////////////////////////////////////////////////////
void AHumanoid::OnEquipWeapon()
{
	HolsterWeapon_Event.Broadcast(m_EquippedWeapon);	
}

/////////////////////////////////////////////////////
void AHumanoid::BeginPlay()
{
	Super::BeginPlay();

	m_MaxBackwardsSpeed = GetCharacterMovement()->MaxWalkSpeed;
	BASComponent->SetSprintingSpeedThreshold(m_SprintingSpeedThreshold);
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
		if (m_bIsAiming)
		{
			ToggleAiming();
		}
		m_SpeedBeforeSprinting = GetCharacterMovement()->MaxWalkSpeed;
		GetCharacterMovement()->MaxWalkSpeed = m_MaxForwardSpeed;
		m_bIsSprinting = true;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = m_SpeedBeforeSprinting;
		m_bIsSprinting = false;
	}

	if (!HasAuthority())
	{
		SetSprinting_Server(m_bIsSprinting);
	}
}

void AHumanoid::SetSprinting_Server_Implementation(bool bWantsToSprint)
{
	SetSprinting(bWantsToSprint);
}

bool AHumanoid::SetSprinting_Server_Validate(bool bWantsToSprint)
{
	return true;
}

/////////////////////////////////////////////////////
void AHumanoid::FireEquippedWeapon()
{
	if (CanFire() && m_EquippedWeapon)
	{
		m_EquippedWeapon->OnFire();
	}
}

void AHumanoid::StopFireEquippedWeapon()
{
	if (m_EquippedWeapon)
	{
		m_EquippedWeapon->OnStopFire();
	}
}

/////////////////////////////////////////////////////
void AHumanoid::SetVelocity(float NewVelocity)
{
	GetCharacterMovement()->MaxWalkSpeed = FMath::Clamp(NewVelocity, -m_MaxBackwardsSpeed, m_MaxForwardSpeed);
	GetCharacterMovement()->MaxWalkSpeedCrouched = FMath::Clamp(NewVelocity, -m_MaxCrouchingSpeed, m_MaxCrouchingSpeed);
	
	if (!HasAuthority())
	{
		SetVelocity_Server(NewVelocity);
	}
}

void AHumanoid::SetVelocity_Server_Implementation(float NewVelocity)
{
	SetVelocity(NewVelocity);
}

bool AHumanoid::SetVelocity_Server_Validate(float NewVelocity)
{
	return true;
}

void AHumanoid::IncrementVelocity(float Increment)
{
	float newMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed + Increment;
	GetCharacterMovement()->MaxWalkSpeed = FMath::Clamp(newMaxWalkSpeed, -m_MaxBackwardsSpeed, m_MaxForwardSpeed);

	float newMaxCrouchingSpeed = GetCharacterMovement()->MaxWalkSpeedCrouched + Increment;
	GetCharacterMovement()->MaxWalkSpeedCrouched = FMath::Clamp(newMaxCrouchingSpeed, -m_MaxCrouchingSpeed, m_MaxCrouchingSpeed);

	if (!HasAuthority())
	{
		IncrementVelocity_Server(Increment);
	}
}

void AHumanoid::IncrementVelocity_Server_Implementation(float Increment)
{
	IncrementVelocity(Increment);
}

bool AHumanoid::IncrementVelocity_Server_Validate(float Increment)
{
	return true;
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
	else if (Value < 0.0f && m_bIsSprinting)
	{
		SetSprinting(false);
	}
}

void AHumanoid::MoveRight(float Value)
{
	if (Controller && Value != 0.0f && !m_bIsSprinting)
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
	AddControllerYawInput(Rate * m_BaseTurnRate * GetWorld()->GetDeltaSeconds());	
}

void AHumanoid::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * m_BaseTurnRate * GetWorld()->GetDeltaSeconds());	
}

/////////////////////////////////////////////////////
void AHumanoid::SetProne(bool bProne)
{
	m_bIsProne = bProne;
}

/////////////////////////////////////////////////////
void AHumanoid::ToggleAiming()
{
	if (!m_bIsSprinting)
	{
		m_bIsAiming = !m_bIsAiming;
		BASComponent->GetActorVariables().bIsAiming = m_bIsAiming;
	}
	else
	{
		m_bIsAiming = false;
		BASComponent->GetActorVariables().bIsAiming = false;
	}
}

/////////////////////////////////////////////////////
void AHumanoid::SetCrouch(bool bCrouch)
{
	if (bCrouch)
	{
		BASComponent->GetActorVariables().MovementAdditive = EMovementAdditive::Crouch;
		Crouch();
	}
	else
	{
		BASComponent->GetActorVariables().MovementAdditive = EMovementAdditive::None;
		UnCrouch();
	}
}

/////////////////////////////////////////////////////
void AHumanoid::ToggleJump()
{
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		BASComponent->OnJumpEvent.Broadcast();
	}
	else
	{
		StopJumping();
	}
}

/////////////////////////////////////////////////////
void AHumanoid::ReloadWeapon()
{
	if (m_EquippedWeapon)
	{
		m_EquippedWeapon->Reload();
	}
}

/////////////////////////////////////////////////////
void AHumanoid::ChangeWeaponFireMode()
{
	if (m_EquippedWeapon)
	{
		m_EquippedWeapon->ToggleFireMode();
	}
}

/////////////////////////////////////////////////////
void AHumanoid::GrabItem(AItemBase* ItemToGrab, bool bKeepRelativeOffset)
{
	m_ItemInHand = ItemToGrab;
	CalcAndSafeActorOffset(ItemToGrab);
	FName handSocket = "HandLeft";
	ItemToGrab->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, handSocket);
	
	if (bKeepRelativeOffset)
	{
		ItemToGrab->AddActorLocalTransform(m_ItemOffset);		
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
	
	m_ItemOffset = offset;
	return offset;
}

/////////////////////////////////////////////////////
AItemBase* AHumanoid::DropItem()
{
	if (m_ItemInHand == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s tried to drop an item without having any to drop!"), *GetName());
		return nullptr;
	}
	AItemBase* itemToDrop = m_ItemInHand;	
	itemToDrop->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	itemToDrop->ShouldSimulatePhysics(true);
	
	m_ItemInHand = nullptr;
	return itemToDrop;
}

/////////////////////////////////////////////////////
bool AHumanoid::CanFire() const
{
	return !m_bIsSprinting && GetCharacterMovement()->IsMovingOnGround() && !m_bComponentBlocksFiring;
}

/////////////////////////////////////////////////////
FName AHumanoid::GetEquippedWeaponAttachPoint() const
{
	return m_EquippedWeaponAttachPoint;
}

/////////////////////////////////////////////////////
void AHumanoid::SetUpDefaultEquipment()
{
	if (m_DefaultGun == nullptr)
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
		m_WeaponToEquip = SpawnWeapon(m_DefaultGun);
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
bool AHumanoid::IsAiming() const
{
	return m_bIsAiming;
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
	FTransform offset = FTransform(m_ItemOffset);
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
	if (m_ItemInHand && !m_ItemInHand->IsAttachedTo(this))
	{
		m_ItemInHand = nullptr;
	}
}


/////////////////////////////////////////////////////
					/* Networking */
/////////////////////////////////////////////////////
void AHumanoid::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHumanoid, m_bIsSprinting);
	DOREPLIFETIME(AHumanoid, m_bIsAiming);
	DOREPLIFETIME(AHumanoid, m_bIsProne);
	DOREPLIFETIME(AHumanoid, m_WeaponToEquip);
}

/////////////////////////////////////////////////////
void AHumanoid::SetWeaponToEquip(AGun* Weapon)
{
	if (HasAuthority())
	{
		m_WeaponToEquip = Weapon;
		OnWeaponEquip();
	}
}

/////////////////////////////////////////////////////
void AHumanoid::OnWeaponEquip()
{
	if(m_WeaponToEquip)
	{
		SetEquippedWeapon(m_WeaponToEquip);
		m_EquippedWeapon->OnEquip(this);
		BASComponent->GetActorVariables().EquippedWeaponType = m_EquippedWeapon->GetWeaponType();
	}
}

/////////////////////////////////////////////////////
void AHumanoid::OnRep_bIsSprining()
{
	GetCharacterMovement()->MaxWalkSpeed = m_bIsSprinting ? m_MaxForwardSpeed : m_SpeedBeforeSprinting;
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
