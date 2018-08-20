// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "Engine/World.h"
#include "Interactable.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Enums/WeaponEnums.h"
#include "Gun.h"
#include "CoopArena.h"
#include "Camera/CameraComponent.h"


/////////////////////////////////////////////////////
APlayerCharacter::APlayerCharacter()
{
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

	CheckForInteractables();
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
	AGun* GunInFocus = Cast<AGun>(_ActorInFocus);
	if (GunInFocus && !_EquippedWeapon)
	{
		GunInFocus->OnEquip(this);
	}	
	EquipWeapon_Event.Broadcast();
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
	return GetWorld()->LineTraceSingleByChannel(outHitresult, cameraLocation, traceEndLoaction, ECC_Item, params);
}


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
		IInteractable::Execute_OnBeginInteract(_ActorInFocus, this, _ComponentInFocus);
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

	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::ToggleJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &APlayerCharacter::ToggleJump);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

	// Bind mouse movement.
	PlayerInputComponent->BindAxis("Turn", this, &APlayerCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerCharacter::LookUpAtRate);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APlayerCharacter::FireEquippedWeapon);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &APlayerCharacter::StopFireEquippedWeapon);

	// Bind crouch event
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APlayerCharacter::ToggleCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &APlayerCharacter::ToggleCrouch);

	// Interact event
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerCharacter::OnBeginInteracting);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &APlayerCharacter::OnEndInteracting);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &APlayerCharacter::ToggleAiming);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &APlayerCharacter::ToggleAiming);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APlayerCharacter::StartSprinting);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APlayerCharacter::StopSprinting);

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


UCameraComponent* APlayerCharacter::GetActiveCamera() const
{
	return _FirstPersonCamera->IsActive() ? _FirstPersonCamera : _ThirdPersonCamera;
}


const FHitResult& APlayerCharacter::GetInteractionLineTraceHitResult() const
{
	return _InteractionHitResult;
}