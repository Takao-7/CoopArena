// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicAnimationSystemComponent.h"
#include "Interfaces/BAS_Interface.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerCharacter.h"


UBasicAnimationSystemComponent::UBasicAnimationSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	_AirControl = 0.0f;
	_BrakingDecleration = 800.0f;
	_MaxAcceleration = 800.0f;
	_CrouchHalfHeight = 65.0f;
	_JumpZVelocity = 300.0f;
	_MovingTurnSpeed = 260.0f;
	_ViewDirectionResetSpeed = 7.5f;

	_MaxCrouchSpeed = 200.0f;
	_MaxWalkSpeed = 200.0f;
	_MaxJogSpeed = 300.0f;
	_MaxSprintSpeed = 600.0f;
}


//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	IBAS_Interface* _BASInterface = Cast<IBAS_Interface>(GetOwner());
	if (_BASInterface == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s does not implement the BAS_Interface!"), *GetOwner()->GetName());
		PrimaryComponentTick.SetTickFunctionEnable(false);
	}
	else
	{
		SetMovementComponentValues();
		SetUseControlRotationYawOnCharacter();
	}
}


//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetInputDirection();
	SetHorizontalVelocity();
	SetMovementType();
	SetIsMovingForward();
	SetAimPitch();
	SetYawActor();
	SetViewDirection(DeltaTime);
	_variables.EquippedWeaponType = IBAS_Interface::Execute_GetEquippedWeaponType(GetOwner());
}


//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetYawActor()
{
	_YawActorLastTick = _YawActor;
	_YawActor = GetOwner()->GetActorRotation().Yaw;
}


void UBasicAnimationSystemComponent::SetViewDirection(float DeltaTime)
{
	if (_variables.MovementType == EMovementType::Idle)
	{
		float deltaYaw = _YawActorLastTick - _YawActor;
		_variables.ViewDirection += deltaYaw;
	}
	else
	{
		/* Do not re-set the view direction instantly. */
		float targetDirection = 0.0f;
		//_variables.ViewDirection > 180.0f ? targetDirection = 360.0f : targetDirection = 0.0f;
		_variables.ViewDirection = FMath::FInterpTo(_variables.ViewDirection, targetDirection, DeltaTime, _ViewDirectionResetSpeed);
	}
}


void UBasicAnimationSystemComponent::SetUseControlRotationYawOnCharacter()
{
	APlayerCharacter* ownerCharacter = Cast<APlayerCharacter>(GetOwner());
	if (ownerCharacter)
	{
		if (_MovementComponent)
		{
			ownerCharacter->bUseControllerRotationYaw = false;
		}
		else
		{
			ownerCharacter->bUseControllerRotationYaw = true;
		}
	}
}


void UBasicAnimationSystemComponent::SetMovementComponentValues()
{
	_MovementComponent = Cast<UCharacterMovementComponent>(GetOwner()->GetComponentByClass(TSubclassOf<UCharacterMovementComponent>()));
	if (_MovementComponent == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No CharacterMovementComponent found."), *GetOwner()->GetName());
		return;
	}

	_MovementComponent->MaxWalkSpeedCrouched = _MaxCrouchSpeed;
	_MovementComponent->MaxWalkSpeed = _MaxWalkSpeed;

	_MovementComponent->bUseControllerDesiredRotation = true;
	_MovementComponent->RotationRate.Yaw = _MovingTurnSpeed;

	_MovementComponent->GetNavAgentPropertiesRef().bCanCrouch = true;
}


void UBasicAnimationSystemComponent::SetAimPitch()
{
	float controlPitch = GetOwner()->GetInstigator()->GetControlRotation().Pitch;
	if (controlPitch >= 90.0f)
	{
		controlPitch -= 360.0f;
	}
	_variables.AimPitch = controlPitch;
}


void UBasicAnimationSystemComponent::SetIsMovingForward()
{
	/*
	 * If the angle between this and the last input direction is very high (optimal 180°),
	 * we are moving at the exact opposite direction, so we don't have to turn around.
	 */
	float inputDirectionDelta = FMath::Abs(_variables.InputDirection - _variables.LastInputDirection);
	if (inputDirectionDelta >= 179.0f)
	{
		_variables.bIsMovingForward = !_variables.bIsMovingForward;
	}
	else
	{
		float xInput = GetMovementInputVectorLocalSpace().X;
		/*
		 * We are moving forward when:
		 * a) The X-Input is positive or
		 * b) The X-Input is 0 and the last input was positive
		 */
		_variables.bIsMovingForward = xInput > 0.0f || (FMath::Abs(xInput) == 0.0f && _variables.bIsMovingForward);
	}
}


FVector UBasicAnimationSystemComponent::GetMovementInputVectorLocalSpace()
{
	FTransform actorTransform = GetOwner()->GetTransform();
	FVector inputVector = GetOwner()->GetInstigator()->GetLastMovementInputVector();
	return actorTransform.InverseTransformVector(inputVector);
}


void UBasicAnimationSystemComponent::SetMovementType()
{
	if (_variables.HorizontalVelocity == 0.0f)
	{
		//if (_variables.MovementType != EMovementType::IdleCrouch && _variables.MovementType != EMovementType::Idle)
		//{
		//	// We just stopped moving.
		//	_variables.ViewDirection = _variables.LastInputDirection;
		//}

		if (IBAS_Interface::Execute_IsCrouching(GetOwner()))
		{
			_variables.MovementType = EMovementType::IdleCrouch;
		}
		else
		{
			_variables.MovementType = EMovementType::Idle;
		}
	}
	else
	{
		if (_variables.HorizontalVelocity <= _MaxCrouchSpeed && IBAS_Interface::Execute_IsCrouching(GetOwner()))
		{
			_variables.MovementType = EMovementType::Crouching;
		}
		else if (_variables.HorizontalVelocity <= _MaxWalkSpeed)
		{
			_variables.MovementType = EMovementType::Walking;
		}
		else if (_variables.HorizontalVelocity <= _MaxJogSpeed)
		{
			_variables.MovementType = EMovementType::Jogging;
		}
		else
		{
			_variables.MovementType = EMovementType::Sprinting;
		}
	}
}


void UBasicAnimationSystemComponent::SetHorizontalVelocity()
{
	FVector velocityVector = GetOwner()->GetVelocity();
	velocityVector.Z = 0.0f;

	_variables.HorizontalVelocity = velocityVector.Size();
}


void UBasicAnimationSystemComponent::SetInputDirection()
{
	FVector lastInputVector = GetOwner()->GetInstigator()->GetLastMovementInputVector();
	FRotator lastInputRotator = lastInputVector.ToOrientationRotator();

	if (lastInputVector.Size() != 0.0f)	// Only set _LastInputDirection if we are moving.
	{
		_variables.LastInputDirection = _variables.InputDirection;
	}

	_variables.InputDirection = (lastInputRotator - GetOwner()->GetInstigator()->GetControlRotation()).Yaw;
}


//////////////////////////////////////////////////////////////////////////////////////
bool UBasicAnimationSystemComponent::IsCrouching_Implementation()
{
	return _variables.MovementType == EMovementType::Crouching ? true : false;
}


bool UBasicAnimationSystemComponent::IsAiming_Implementation()
{
	return _variables.bIsAiming;
}


EWEaponType UBasicAnimationSystemComponent::GetEquippedWeaponType_Implementation()
{
	return _variables.EquippedWeaponType;
}


//////////////////////////////////////////////////////////////////////////////////////
FBASVariables UBasicAnimationSystemComponent::GetActorVariables() const
{
	return _variables;
}