// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "Engine/World.h"
#include "Interactable.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "Components/BasicAnimationSystemComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Enums/WeaponEnums.h"
#include "Gun.h"
#include "CoopArena.h"
#include "Camera/CameraComponent.h"
#include "CoopArenaGameMode.h"


/////////////////////////////////////////////////////
APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	_InteractionRange = 200.0f;
	_InteractableInFocus = nullptr;

	_FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("First person camera"));
	_FirstPersonCamera->SetupAttachment(GetMesh(), "head");
	_FirstPersonCamera->bUsePawnControlRotation = true;
	_FirstPersonCamera->SetRelativeLocationAndRotation(FVector(7.0f, 5.0f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));

	_SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring arm"));
	_SpringArm->SetupAttachment(RootComponent);
	_SpringArm->SetRelativeLocation(FVector(0.0f, 20.0f, 60.0f));
	_SpringArm->TargetArmLength = 200.0f;
	_SpringArm->bUsePawnControlRotation = true;
	_SpringArm->bEnableCameraLag = true;
	_SpringArm->bEnableCameraRotationLag = true;

	_ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Third person camera"));
	_ThirdPersonCamera->SetupAttachment(_SpringArm, "SpringEndpoint");

	m_IncrementVelocityAmount = 50.0f;

	m_TeamName = "Player Team";
}

/////////////////////////////////////////////////////
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocallyControlled() && InputEnabled())
	{
		CheckForInteractables();
	}
}

/////////////////////////////////////////////////////
void APlayerCharacter::CheckForInteractables()
{
	if (IsPlayerControlled() && InteractionLineTrace(_InteractionHitResult))
	{
		AActor* hitActor = _InteractionHitResult.GetActor();
		UPrimitiveComponent* hitComponent = _InteractionHitResult.GetComponent();
		IInteractable* interactable = Cast<IInteractable>(hitActor);
		if (interactable)
		{
			// Only do Begin/End line traces calls if we are pointing at something new
			if (interactable != _InteractableInFocus)
			{
				if (_ActorInFocus)
				{
					IInteractable::Execute_OnEndLineTraceOver(_ActorInFocus, this);
				}

				IInteractable::Execute_OnBeginLineTraceOver(hitActor, this, hitComponent);				
				SetActorInFocus(hitActor);
			}
			SetComponentInFocus(hitComponent);
		}
	}
	else if (_InteractableInFocus && _ActorInFocus)
	{
		IInteractable::Execute_OnEndLineTraceOver(_ActorInFocus, this);
		SetActorInFocus(nullptr);
		SetComponentInFocus(nullptr);
	}
}

/////////////////////////////////////////////////////
void APlayerCharacter::OnHolsterWeapon()
{
	HolsterWeapon_Event.Broadcast(m_EquippedWeapon, -1);
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		ACoopArenaGameMode* gameMode = GetWorld()->GetAuthGameMode<ACoopArenaGameMode>();
		gameMode->RegisterPlayerCharacter(this);

		OnDestroyed.AddDynamic(this, &APlayerCharacter::HandleOnDestroy);
	}
}

/////////////////////////////////////////////////////
void APlayerCharacter::HandleOnDestroy(AActor* DestroyedActor)
{
	AGun* equippedGun = GetEquippedGun();
	if(equippedGun)
	{
		equippedGun->Unequip(true, true);
	}

	ACoopArenaGameMode* gameMode = GetWorld()->GetAuthGameMode<ACoopArenaGameMode>();
	gameMode->UnregisterPlayerCharacter(this);
}

/////////////////////////////////////////////////////
void APlayerCharacter::SetActorInFocus(AActor* actor)
{
	_InteractableInFocus = Cast<IInteractable>(actor);
	if (_InteractableInFocus)
	{
		_ActorInFocus = actor;
	}
	else
	{
		_ActorInFocus = nullptr;
	}	
}


void APlayerCharacter::SetComponentInFocus(UPrimitiveComponent* Component)
{
	_ComponentInFocus = Component;
}

/////////////////////////////////////////////////////
bool APlayerCharacter::InteractionLineTrace(FHitResult& outHitresult)
{
	FVector cameraLocation = GetActiveCamera()->GetComponentLocation();
	FVector forwardVector = GetActiveCamera()->GetForwardVector();
	FVector traceEndLoaction = cameraLocation + forwardVector * _InteractionRange;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	if (m_EquippedWeapon)
	{
		params.AddIgnoredActor((AActor*)m_EquippedWeapon);
	}
	return GetWorld()->LineTraceSingleByChannel(outHitresult, cameraLocation, traceEndLoaction, ECC_Interactable, params);
}

/////////////////////////////////////////////////////
void APlayerCharacter::OnPronePressed()
{
	if (m_bToggleProne)
	{
		m_bIsProne = !m_bIsProne;
	}
	else
	{
		m_bIsProne = true;
	}
}

void APlayerCharacter::OnProneReleased()
{
	if (!m_bToggleProne)
	{
		m_bIsProne = false;
	}
}

/////////////////////////////////////////////////////
void APlayerCharacter::OnSprintPressed()
{
	if (m_bToggleSprinting)
	{
		SetSprinting(!m_bIsSprinting);
	}
	else if (!m_bIsSprinting)
	{
		SetSprinting(true);
	}
}

void APlayerCharacter::OnSprintReleased()
{
	if (!m_bToggleSprinting)
	{
		SetSprinting(false);
	}
}

/////////////////////////////////////////////////////
void APlayerCharacter::OnCrouchPressed()
{
	if (m_bToggleCrouching)
	{
		SetCrouch(!bIsCrouched);
	}
	else
	{
		BASComponent->GetActorVariables().MovementAdditive = EMovementAdditive::Crouch;
		Crouch();
	}
}

void APlayerCharacter::OnCrouchReleased()
{
	if (!m_bToggleCrouching)
	{
		BASComponent->GetActorVariables().MovementAdditive = EMovementAdditive::None;
		UnCrouch();
	}
}

/////////////////////////////////////////////////////
void APlayerCharacter::OnIncreaseVelocity()
{
	IncrementVelocity(m_IncrementVelocityAmount);
}

void APlayerCharacter::OnDecreaseVelocity()
{
	IncrementVelocity(-m_IncrementVelocityAmount);
}

/////////////////////////////////////////////////////
void APlayerCharacter::OnChangeCameraPressed()
{
	_FirstPersonCamera->ToggleActive();
	_ThirdPersonCamera->ToggleActive();

	if (_ThirdPersonCamera->IsActive())
	{
		_InteractionRange += _SpringArm->TargetArmLength;
	}
	else
	{
		_InteractionRange -= _SpringArm->TargetArmLength;
	}
}

/////////////////////////////////////////////////////
void APlayerCharacter::OnAimingPressed()
{
	if (m_EquippedWeapon && !m_bIsSprinting)
	{
		m_bIsAiming = true;
		BASComponent->GetActorVariables().bIsAiming = true;
		Cast<APlayerController>(GetController())->SetViewTargetWithBlend(m_EquippedWeapon, 0.2f);
	}
}

void APlayerCharacter::OnAimingReleased()
{
	if (m_EquippedWeapon && !m_bIsSprinting)
	{
		m_bIsAiming = false;
		BASComponent->GetActorVariables().bIsAiming = false;
		APlayerController* pc = Cast<APlayerController>(GetController());
		pc->SetViewTargetWithBlend(pc->GetPawn(), 0.2f);
	}
}

/////////////////////////////////////////////////////
void APlayerCharacter::OnBeginInteracting()
{
	if (_InteractableInFocus)
	{
		Server_OnBeginInteracting(_ActorInFocus, _ComponentInFocus);
	}
}

void APlayerCharacter::OnEndInteracting()
{
	if (_InteractableInFocus)
	{
		IInteractable::Execute_OnEndInteract(_ActorInFocus, this);
	}
}

/////////////////////////////////////////////////////
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::ToggleJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &APlayerCharacter::ToggleJump);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APlayerCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APlayerCharacter::FireEquippedWeapon);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &APlayerCharacter::StopFireEquippedWeapon);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APlayerCharacter::OnCrouchPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &APlayerCharacter::OnCrouchReleased);

	PlayerInputComponent->BindAction("Prone", IE_Pressed, this, &APlayerCharacter::OnPronePressed);
	PlayerInputComponent->BindAction("Prone", IE_Released, this, &APlayerCharacter::OnProneReleased);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayerCharacter::OnSprintPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerCharacter::OnSprintReleased);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerCharacter::OnBeginInteracting);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &APlayerCharacter::OnEndInteracting);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &APlayerCharacter::OnAimingPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &APlayerCharacter::OnAimingReleased);
	
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &APlayerCharacter::ReloadWeapon);
	PlayerInputComponent->BindAction("ChangeFireMode", IE_Pressed, this, &APlayerCharacter::ChangeWeaponFireMode);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &APlayerCharacter::OnHolsterWeapon);	

	PlayerInputComponent->BindAction("Increase velocity", IE_Pressed, this, &APlayerCharacter::OnIncreaseVelocity);
	PlayerInputComponent->BindAction("Decrease velocity", IE_Pressed, this, &APlayerCharacter::OnDecreaseVelocity);

	PlayerInputComponent->BindAction("ChangeCamera", IE_Pressed, this, &APlayerCharacter::OnChangeCameraPressed);
}


/////////////////////////////////////////////////////
FVector APlayerCharacter::GetCameraLocation() const
{
	return GetActiveCamera()->GetComponentLocation();
}

/////////////////////////////////////////////////////
UCameraComponent* APlayerCharacter::GetActiveCamera() const
{
	return _FirstPersonCamera->IsActive() ? _FirstPersonCamera : _ThirdPersonCamera;
}

/////////////////////////////////////////////////////
void APlayerCharacter::SetThirdPersonCameraToActive()
{
	_FirstPersonCamera->SetActive(false);
	_ThirdPersonCamera->SetActive(true);
}

/////////////////////////////////////////////////////
const FHitResult& APlayerCharacter::GetInteractionLineTraceHitResult() const
{
	return _InteractionHitResult;
}

/////////////////////////////////////////////////////
void APlayerCharacter::Server_OnBeginInteracting_Implementation(AActor* ActorInFocus, UPrimitiveComponent* ComponentInFocus)
{
	IInteractable::Execute_OnBeginInteract(ActorInFocus, this, ComponentInFocus);
}

bool APlayerCharacter::Server_OnBeginInteracting_Validate(AActor* ActorInFocus, UPrimitiveComponent* ComponentInFocus)
{
	return true;
}

/////////////////////////////////////////////////////
void APlayerCharacter::Server_OnEndInteracting_Implementation(AActor* ActorInFocus)
{
	IInteractable::Execute_OnEndInteract(ActorInFocus, this);
}

bool APlayerCharacter::Server_OnEndInteracting_Validate(AActor* ActorInFocus)
{
	return true;
}