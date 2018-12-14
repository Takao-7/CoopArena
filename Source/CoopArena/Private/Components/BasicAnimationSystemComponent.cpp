#include "BasicAnimationSystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "PlayerCharacter.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/HealthComponent.h"
#include "UnrealNetwork.h"


UBasicAnimationSystemComponent::UBasicAnimationSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bReplicates = true;
	bAutoActivate = true;

	m_TurnSpeed = 10.0f;

	m_Variables.MovementType = EMovementType::Idle;
	m_Variables.MovementAdditive = EMovementAdditive::None;
	m_Variables.EquippedWeaponType = EWEaponType::None;

	m_180TurnThreshold = 120.0f;
	m_AngleClampThreshold = 180.0f;
	m_180TurnPredictionTime = 0.2f;
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::BeginPlay()
{
	Super::BeginPlay();	

	SetIsLocallyControlled();
	OnJumpEvent.AddDynamic(this, &UBasicAnimationSystemComponent::PlayJumpAnimation);	
	FindAnimInstance();
	FindCharacterMovementComponent();

	UHealthComponent* healthComp = Cast<UHealthComponent>(GetOwner()->GetComponentByClass(UHealthComponent::StaticClass()));
	if (healthComp)
	{
		healthComp->OnDeath.AddDynamic(this, &UBasicAnimationSystemComponent::DisableComponent);
	}
	
	m_ActorYawLastFrame = GetOwner()->GetActorRotation().Yaw;
	m_CapsuleHalfHeight = _Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetIsLocallyControlled()
{
	if(_bIsLocallyControlled == false)
	{
		APawn* instigator = GetOwner()->GetInstigator();
		_bIsLocallyControlled = instigator ? instigator->IsLocallyControlled() : false;

		if (_bIsLocallyControlled)
		{
			OnJumpEvent.AddDynamic(this, &UBasicAnimationSystemComponent::BroadcastJumpEvent_Server);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::FindAnimInstance()
{
	_Owner = Cast<ACharacter>(GetOwner());
	check(_Owner);

	m_AnimInstance = _Owner->GetMesh()->GetAnimInstance();
	check(m_AnimInstance);
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::FindCharacterMovementComponent()
{
	m_MovementComponent = _Owner->GetCharacterMovement();
	if (m_MovementComponent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' does not have a CharacterMovementComponent."), *GetOwner()->GetName());
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetHorizontalVelocity();
	SetIsMovingForward();
	SetMovementType();
	SetMovementAdditive();
	SetAimYaw(DeltaTime);
	SetAimPitch();
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetHorizontalVelocity()
{
	m_Variables.Velocity = GetOwner()->GetVelocity();

	FVector velocity_Vector = m_Variables.Velocity;
	velocity_Vector.Z = 0.0f;
	float horizontalVelocity = velocity_Vector.Size();

	const FTransform actorTransform = GetOwner()->GetActorTransform();
	const FVector velocity_ActorSpace = actorTransform.InverseTransformVector(velocity_Vector);

	const float yawDelta = velocity_ActorSpace.ToOrientationRotator().Yaw;
	horizontalVelocity *= FMath::Abs(yawDelta) > 120.0f ? -1.0f : 1.0f;

	horizontalVelocity = FMath::IsNearlyZero(horizontalVelocity, 0.1f) ? 0.0f : horizontalVelocity;
	m_Variables.HorizontalVelocity = horizontalVelocity;
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetIsMovingForward()
{
	if (m_Variables.HorizontalVelocity != 0.0f)
	{
		m_Variables.bIsMovingForward = m_Variables.HorizontalVelocity > 0.0f ? true : false;
	}	
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetMovementAdditive()
{
	m_Variables.MovementAdditive = _Owner->bIsCrouched ? EMovementAdditive::Crouch : EMovementAdditive::None;
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetAimYaw(float DeltaTime)
{
	if (!_bIsLocallyControlled)
	{
		m_AimYawLastFrame = m_Variables.AimYaw;
		m_Variables.AimYaw = m_AimYaw;
		CheckWhetherToPlayTurnAnimation(DeltaTime, m_Variables.AimYaw);
		CheckIfTurnAnimFinished();
		return;
	}

	const FRotator actorRotation = GetOwner()->GetActorRotation();
	float yawDelta = 0.0f;
	if (m_Variables.MovementType == EMovementType::Idle)
	{
		yawDelta = m_ActorYawLastFrame - actorRotation.Yaw;
		yawDelta = MapAngleTo180_Forced(yawDelta);
		float newAimYaw = m_Variables.AimYaw + yawDelta;

		CheckWhetherToPlayTurnAnimation(DeltaTime, newAimYaw);
		AddCurveValueToYawWhenTurning(newAimYaw);

		m_Variables.AimYaw = MapAngleTo180(newAimYaw);
	}
	else
	{
		FVector velocity = m_Variables.Velocity;
		velocity.Z = 0.0f;
		velocity *= m_Variables.bIsMovingForward ? 1.0f : -1.0f;

		const FTransform actorTransform = GetOwner()->GetActorTransform();
		yawDelta = actorTransform.InverseTransformVector(velocity).ToOrientationRotator().Yaw;
		const float newAimYaw = FMath::FInterpTo(m_Variables.AimYaw, yawDelta, DeltaTime, m_TurnSpeed);
		m_Variables.AimYaw = MapAngleTo180(newAimYaw);
	}

	m_ActorYawLastFrame = actorRotation.Yaw;
	m_AimYawLastFrame = m_Variables.AimYaw;

	if (GetOwner()->HasAuthority() == false)
	{
		ReplicateAimYaw_Server(m_Variables.AimYaw);
	}
	else
	{
		m_AimYaw = m_Variables.AimYaw;
	}

	CheckIfTurnAnimFinished();
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::AddCurveValueToYawWhenTurning(float& Yaw)
{
	if (TurnAnimationIsActive())
	{
		const float curveValue = m_AnimInstance->GetCurveValue("DistanceCurve");
		if (m_bTurnCurveIsPlaying)
		{
			const float curveDelta = m_CurveValueLastFrame - curveValue;
			Yaw += curveDelta;
			m_CurveValueLastFrame = curveValue;	
		}
		else if (curveValue)
		{
			/* The first time we got a value from the curve, we want to just set the last curve value and not update delta yaw. */
			m_CurveValueLastFrame = curveValue;
			m_bTurnCurveIsPlaying = true;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::CheckWhetherToPlayTurnAnimation(float DeltaTime, float NewAimYaw)
{
	const float aimYaw_Abs = FMath::Abs(NewAimYaw);
	if (m_TurnAnimationPlaying || aimYaw_Abs < 90.0f)
	{
		return;
	}

	const float aimYawDelta = NewAimYaw - m_AimYawLastFrame;
	const float predictedYaw = (aimYawDelta * (m_180TurnPredictionTime / DeltaTime)) + NewAimYaw;
	// Play the turn animation faster to avoid over bending the upper body.
	const float playRate = FMath::Abs(predictedYaw) > (m_180TurnThreshold * 1.5f) ? 1.5f : 1.0f;

	if (NewAimYaw <= -90.0f) // Turn right
	{
		m_TurnAnimationPlaying = predictedYaw <= -m_180TurnThreshold && m_TurnRight180Animation ? m_TurnRight180Animation : m_TurnRight90Animation;	
		m_bIsTurningRight = true;
	}
	else  // Turn left
	{
		m_TurnAnimationPlaying = predictedYaw >= m_180TurnThreshold && m_TurnLeft180Animation ? m_TurnLeft180Animation : m_TurnLeft90Animation;
		m_bIsTurningRight = false;
	}

	m_bTurnCurveIsPlaying = false;
	m_AnimInstance->Montage_Play(m_TurnAnimationPlaying, playRate, EMontagePlayReturnType::MontageLength, 0.0f, false);
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::DisableComponent(AActor* Actor, AController* Controller, AController* Killer)
{
	PrimaryComponentTick.SetTickFunctionEnable(false);
	_bIsLocallyControlled = false;
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::CheckIfTurnAnimFinished()
{
	if(TurnAnimationIsActive() == false)
	{
		m_TurnAnimationPlaying = nullptr;
		m_bTurnCurveIsPlaying = false;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
bool UBasicAnimationSystemComponent::TurnAnimationIsActive()
{
	return m_AnimInstance->Montage_IsActive(m_TurnAnimationPlaying) && m_TurnAnimationPlaying;
}

//////////////////////////////////////////////////////////////////////////////////////
float UBasicAnimationSystemComponent::MapAngleTo180_Forced(float Angle)
{
	float mappedAngle = Angle;
	while (mappedAngle >= m_AngleClampThreshold)
	{
		mappedAngle -= 360.0f;
	}

	while (mappedAngle <= -m_AngleClampThreshold)
	{
		mappedAngle += 360.0f;
	}

	return mappedAngle;
}

float UBasicAnimationSystemComponent::MapAngleTo180(float Angle)
{
	const bool bIsTurning = m_AnimInstance->GetCurveValue("IsTurning");
	if ((bIsTurning || m_TurnAnimationPlaying) && FMath::Abs(Angle) < 360.0f)
	{
		return Angle;
	}
	else
	{
		return MapAngleTo180_Forced(Angle);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::PlayJumpAnimation()
{
	if (m_Variables.HorizontalVelocity != 0.0f)
	{
		if (!m_AnimInstance->Montage_IsPlaying(m_MovingJumpAnimation))
		{
			m_AnimInstance->Montage_Play(m_MovingJumpAnimation);
		}
	}
	else if (!m_AnimInstance->Montage_IsPlaying(m_IdleJumpAnimation))
	{
		m_AnimInstance->Montage_Play(m_IdleJumpAnimation);
	}	
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetAimPitch()
{
	if(_bIsLocallyControlled)
	{
		const APawn* instigator = GetOwner()->GetInstigator();
		float controlPitch = instigator->GetControlRotation().Pitch;
		if (controlPitch >= 90.0f)
		{
			controlPitch -= 360.0f;
		}
		m_Variables.AimPitch = controlPitch;
		SetAimPitch_Server(controlPitch);
	}
}

void UBasicAnimationSystemComponent::SetAimPitch_Server_Implementation(float AimPitch)
{
	SetAimPitch_Multicast(AimPitch);
}

bool UBasicAnimationSystemComponent::SetAimPitch_Server_Validate(float AimPitch)
{
	return true;
}

void UBasicAnimationSystemComponent::SetAimPitch_Multicast_Implementation(float AimPitch)
{
	m_Variables.AimPitch = AimPitch;
}

//////////////////////////////////////////////////////////////////////////////////////
FBASVariables& UBasicAnimationSystemComponent::GetActorVariables()
{
	return m_Variables;
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetMovementType()
{
	m_Variables.MovementType_LastFrame = m_Variables.MovementType;

	if (!FMath::IsNearlyZero(m_Variables.Velocity.Z))
	{
		m_Variables.MovementType = EMovementType::InAir;
	}
	else if (m_Variables.HorizontalVelocity == 0.0f)
	{
		m_Variables.MovementType = EMovementType::Idle;
	}
	else
	{
		m_Variables.MovementType = m_Variables.HorizontalVelocity > m_SprintingSpeedThreshold ? EMovementType::Sprinting : EMovementType::Moving;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetMovmentAdditive()
{
	if (m_MovementComponent)
	{
		m_Variables.MovementAdditive = m_MovementComponent->IsCrouching() ? EMovementAdditive::Crouch : EMovementAdditive::None;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetSprintingSpeedThreshold(float SprintingSpeedThreshold)
{
	m_SprintingSpeedThreshold = SprintingSpeedThreshold;
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::ReplicateAimYaw_Server_Implementation(float AimYaw)
{
	m_AimYaw = AimYaw;
}

bool UBasicAnimationSystemComponent::ReplicateAimYaw_Server_Validate(float AimYaw)
{
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::BroadcastJumpEvent_Server_Implementation()
{
	BroadcastJumpEvent_Multicast();
}

bool UBasicAnimationSystemComponent::BroadcastJumpEvent_Server_Validate()
{
	return true;
}

void UBasicAnimationSystemComponent::BroadcastJumpEvent_Multicast_Implementation()
{
	if (!_bIsLocallyControlled)
	{
		OnJumpEvent.Broadcast();
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UBasicAnimationSystemComponent, m_AimYaw, COND_SkipOwner);
}