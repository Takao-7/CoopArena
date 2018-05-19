// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "Engine/World.h"
#include "Interactable.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
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

	if (IsPlayerControlled() && InteractionLineTrace(_InteractionHitResult))
	{
		AActor* hitActor = _InteractionHitResult.GetActor();
		IInteractable* interactable = Cast<IInteractable>(hitActor);
		if (interactable)
		{
			/*if (_InteractionHitResult.GetActor())
			{
				UE_LOG(LogTemp, Warning, TEXT("Hit the item: %s"), *hitActor->GetName());
			}*/
			IInteractable::Execute_OnBeginLineTraceOver(hitActor, this);

			if(interactable != _InteractableInFocus && _ActorInFocus)
			{
				IInteractable::Execute_OnEndLineTraceOver(_ActorInFocus, this);
			}
			_InteractableInFocus = interactable;
			_ActorInFocus = hitActor;
		}
		/*else if(_InteractionHitResult.GetActor())
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit the actor: %s"), *hitActor->GetName());
		}*/
	}
	else if (_InteractableInFocus && _ActorInFocus)
	{
		IInteractable::Execute_OnEndLineTraceOver(_ActorInFocus, this);
		_InteractableInFocus = nullptr;
		_ActorInFocus = nullptr;
	}
}


/////////////////////////////////////////////////////
bool APlayerCharacter::InteractionLineTrace(FHitResult& outHitresult)
{
	FVector cameraLocation = GetActiveCamera()->GetComponentLocation();
	FVector forwardVector = GetActiveCamera()->GetForwardVector();
	FVector traceEndLoaction = cameraLocation + forwardVector * _InteractionRange;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	return GetWorld()->LineTraceSingleByChannel(outHitresult, cameraLocation, traceEndLoaction, ECC_Item, params);
}


/////////////////////////////////////////////////////
void APlayerCharacter::OnBeginInteracting()
{
	if (_InteractableInFocus && _ActorInFocus)
	{
		IInteractable::Execute_OnBeginInteract(_ActorInFocus, this);
	}
}


void APlayerCharacter::OnEndInteracting()
{
	if (_InteractableInFocus && _ActorInFocus)
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
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

	// Bind mouse movement.
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APlayerCharacter::FireEquippedWeapon);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &APlayerCharacter::StopFireEquippedWeapon);

	// Bind crouch event
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APlayerCharacter::ToggleCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &APlayerCharacter::ToggleCrouch);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerCharacter::OnBeginInteracting);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &APlayerCharacter::OnEndInteracting);
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