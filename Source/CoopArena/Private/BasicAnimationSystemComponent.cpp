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
	_IdleTurnAngleThreshold = 90.0f;

	_MaxCrouchSpeed = 200.0f;
	_MaxWalkSpeed = 200.0f;
	_MaxJogSpeed = 300.0f;
	_MaxSprintSpeed = 600.0f;

	_variables.MovementType = EMovementType::Idle;
	_variables.MovementAdditive = EMovementAdditive::None;
	_variables.EquippedWeaponType = EWEaponType::None;
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
	_variables.EquippedWeaponType = IBAS_Interface::Execute_GetEquippedWeaponType(GetOwner());
}


//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetInputDirection()
{
	_lastInputVector = GetOwner()->GetInstigator()->GetLastMovementInputVector();
	if (_lastInputVector.Size() == 0.0f)	// Only set input direction if we are moving.
	{
		return;
	}

	FRotator lastInputRotator = _lastInputVector.ToOrientationRotator();
	FRotator controlRotation = GetOwner()->GetInstigator()->GetControlRotation();
	FRotator deltaRotation = lastInputRotator - controlRotation;
	deltaRotation.Normalize();

	_variables.LastInputDirection = _variables.InputDirection;
	_variables.InputDirection = deltaRotation.Yaw;
}


void UBasicAnimationSystemComponent::SetHorizontalVelocity()
{
	FVector velocityVector = GetOwner()->GetVelocity();
	velocityVector.Z = 0.0f;

	_variables.HorizontalVelocity = velocityVector.Size();
}


void UBasicAnimationSystemComponent::SetMovementType()
{
	_variables.MovementType = IBAS_Interface::Execute_GetMovementType(GetOwner());
	_variables.MovementAdditive = IBAS_Interface::Execute_GetMovementAdditive(GetOwner());
}


void UBasicAnimationSystemComponent::SetIsMovingForward()
{
	if (_lastInputVector.Size() == 0.0f)	// Only set movement direction if we are moving.
	{
		return;
	}
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


void UBasicAnimationSystemComponent::SetAimPitch()
{
	float controlPitch = GetOwner()->GetInstigator()->GetControlRotation().Pitch;
	if (controlPitch >= 90.0f)
	{
		controlPitch -= 360.0f;
	}
	_variables.AimPitch = controlPitch;
}


void UBasicAnimationSystemComponent::SetYawActor()
{
	_YawActorLastTick = _YawActor;
	_YawActor = GetOwner()->GetActorRotation().Yaw;
}


//////////////////////////////////////////////////////////////////////////////////////
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
	_MovementComponent = Cast<UCharacterMovementComponent>(GetOwner()->FindComponentByClass<UCharacterMovementComponent>());
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


FVector UBasicAnimationSystemComponent::GetMovementInputVectorLocalSpace()
{
	FTransform actorTransform = GetOwner()->GetTransform();
	return actorTransform.InverseTransformVector(_lastInputVector);
}


//////////////////////////////////////////////////////////////////////////////////////
bool UBasicAnimationSystemComponent::IsAiming_Implementation()
{
	return _variables.bIsAiming;
}


EWEaponType UBasicAnimationSystemComponent::GetEquippedWeaponType_Implementation()
{
	return _variables.EquippedWeaponType;
}


EMovementType UBasicAnimationSystemComponent::GetMovementType_Implementation()
{
	return _variables.MovementType;
}


EMovementAdditive UBasicAnimationSystemComponent::GetMovementAdditive_Implementation()
{
	return _variables.MovementAdditive;
}


//////////////////////////////////////////////////////////////////////////////////////
FBASVariables UBasicAnimationSystemComponent::GetActorVariables() const
{
	return _variables;
}