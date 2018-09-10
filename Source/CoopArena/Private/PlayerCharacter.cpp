// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "Engine/World.h"
#include "Interactable.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Enums/WeaponEnums.h"
#include "Gun.h"
#include "CoopArena.h"
#include "Camera/CameraComponent.h"


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


void APlayerCharacter::OnEquipWeapon()
{
	HolsterWeapon_Event.Broadcast(_EquippedWeapon);
<<<<<<< HEAD
	
	/*AGun* GunInFocus = Cast<AGun>(_ActorInFocus);
	if (GunInFocus && !_EquippedWeapon)
	{
		GunInFocus->OnEquip(this);
	}*/	
=======
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
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
	if (_EquippedWeapon)
	{
		params.AddIgnoredActor((AActor*)_EquippedWeapon);
	}
	return GetWorld()->LineTraceSingleByChannel(outHitresult, cameraLocation, traceEndLoaction, ECC_Interactable, params);
}


void APlayerCharacter::OnPronePressed()
{
	if (bToggleProne)
	{
		bIsProne = !bIsProne;
	}
	else
	{
		bIsProne = true;
	}
}


void APlayerCharacter::OnProneReleased()
{
	if (!bToggleProne)
	{
		bIsProne = false;
	}
}


void APlayerCharacter::OnSprintPressed()
{
	if (bToggleSprinting)
	{
		SetSprinting(!bIsSprinting);
	}
<<<<<<< HEAD
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = _MaxSprintSpeed;
		bIsSprinting = true;
	}
}


void APlayerCharacter::OnSprintReleased()
{
	if (!bToggleSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = _MaxWalkingSpeed;
		bIsSprinting = false;
	}
}


void APlayerCharacter::OnCrouchPressed()
{
	if (bToggleCrouching)
	{
		SetCrouch(!bIsCrouched);
	}
	else
	{
		Crouch();
	}
}


void APlayerCharacter::OnCrouchReleased()
{
	if (!bToggleCrouching)
	{
		UnCrouch();
=======
	else if (!bIsSprinting)
	{
		SetSprinting(true);
>>>>>>> 17f86cef60dd7dd576fc030497f09716282c8ed8
	}
}


void APlayerCharacter::OnSprintReleased()
{
	if (!bToggleSprinting)
	{
		SetSprinting(false);
	}
}


void APlayerCharacter::OnCrouchPressed()
{
	if (bToggleCrouching)
	{
		SetCrouch(!bIsCrouched);
	}
	else
	{
		Crouch();
	}
}


void APlayerCharacter::OnCrouchReleased()
{
	if (!bToggleCrouching)
	{
		UnCrouch();
	}
}

/////////////////////////////////////////////////////
void APlayerCharacter::ToggleAiming()
{
	if (bIsAiming)
	{
		bIsAiming = false;
		Cast<APlayerController>(GetController())->SetViewTargetWithBlend(GetController()->GetPawn(), 0.2f);		
	}
	else
	{
		bIsAiming = true;		
		if (_EquippedWeapon)
		{
			Cast<APlayerController>(GetController())->SetViewTargetWithBlend(_EquippedWeapon, 0.2f);
		}
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
// Called to bind functionality to input
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

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &APlayerCharacter::ToggleAiming);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &APlayerCharacter::ToggleAiming);
	
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &APlayerCharacter::ReloadWeapon);
	PlayerInputComponent->BindAction("ChangeFireMode", IE_Pressed, this, &APlayerCharacter::ChangeWeaponFireMode);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &APlayerCharacter::OnEquipWeapon);	
}


/////////////////////////////////////////////////////
FVector APlayerCharacter::GetCameraLocation() const
{
	UCameraComponent* camera = Cast<UCameraComponent>(GetComponentByClass(UCameraComponent::StaticClass()));
	if (camera)
	{
		return camera->GetComponentLocation();
	}
	else
	{
		return FVector::ZeroVector;
	}
}

/////////////////////////////////////////////////////
UCameraComponent* APlayerCharacter::GetActiveCamera() const
{
	return _FirstPersonCamera->IsActive() ? _FirstPersonCamera : _ThirdPersonCamera;
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