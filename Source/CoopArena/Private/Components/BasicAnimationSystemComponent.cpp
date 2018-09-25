#include "BasicAnimationSystemComponent.h"
#include "Interfaces/BAS_Interface.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerCharacter.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "UnrealNetwork.h"


UBasicAnimationSystemComponent::UBasicAnimationSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PostPhysics;

	m_TurnSpeed = 10.0f;

	m_Variables.MovementType = EMovementType::Idle;
	m_Variables.MovementAdditive = EMovementAdditive::None;
	m_Variables.EquippedWeaponType = EWEaponType::None;

	bReplicates = true;
	bAutoActivate = true;

	m_180TurnThreshold = 135.0f;
	m_AngleClampThreshold = 180.0f;
	m_180TurnPredictionTime = 0.2f;
	m_TurnAnimPlayRate = 1.0f;
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
void UBasicAnimationSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	FindAnimInstance();
	FindCharacterMovementComponent();
	
	m_ActorYawLastFrame = GetOwner()->GetActorRotation().Yaw;
	m_CapsuleHalfHeight = m_Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	OnJumpEvent.AddDynamic(this, &UBasicAnimationSystemComponent::PlayJumpAnimation);

	if (GetWorld()->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		PrimaryComponentTick.SetTickFunctionEnable(false);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::FindAnimInstance()
{
	m_Owner = Cast<ACharacter>(GetOwner());
	if (m_Owner)
	{
		m_AnimInstance = m_Owner->GetMesh()->GetAnimInstance();
	}
	else
	{
		UE_LOG(LogTemp, Fatal, TEXT("'%s' does not have an animation instance!"), *GetOwner()->GetName());
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::FindCharacterMovementComponent()
{
	ACharacter* character = Cast<ACharacter>(GetOwner());
	if (character)
	{
		m_MovementComponent = character->GetCharacterMovement();
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::CheckIfLocallyControlled()
{
	bool bIsLocallyControlled = GetOwner()->GetInstigator()->IsLocallyControlled();
	if (!bIsLocallyControlled)
	{
		PrimaryComponentTick.SetTickFunctionEnable(false);
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
	FVector velocity = GetOwner()->GetVelocity();
	velocity.Z = 0.0f;
	float horizontalVelocity = velocity.Size();

	const FTransform actorTransform = GetOwner()->GetActorTransform();
	const FVector velocityActorSpace = actorTransform.InverseTransformVector(velocity);

	const float yawDelta = velocityActorSpace.ToOrientationRotator().Yaw;
	horizontalVelocity *= FMath::Abs(yawDelta) > 120.0f ? -1.0f : 1.0f;

	horizontalVelocity = FMath::IsNearlyZero(horizontalVelocity, 0.1f) ? 0.0f : horizontalVelocity;
	m_Variables.HorizontalVelocity = horizontalVelocity;
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetIsMovingForward()
{
	if (m_Variables.HorizontalVelocity == 0.0f)
	{
		return;
	}

	m_Variables.bIsMovingForward = m_Variables.HorizontalVelocity > 0.0f ? true : false;
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetMovementAdditive()
{
	if (GetOwner()->Role != ROLE_AutonomousProxy)
	{
		m_Variables.MovementAdditive = m_Owner->bIsCrouched ? EMovementAdditive::Crouch : EMovementAdditive::None;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetAimYaw(float DeltaTime)
{
	if (GetOwner()->Role != ENetRole::ROLE_AutonomousProxy)
	{
		m_AimYawLastFrame = m_Variables.AimYaw;
		m_Variables.AimYaw = m_AimYaw;
		CheckWhetherToPlayTurnAnimation(DeltaTime, m_Variables.AimYaw);
		CheckIfTurnAnimFinished();
		return;
	}

	FRotator actorRotation = GetOwner()->GetActorRotation();
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
		FVector velocity = GetOwner()->GetVelocity();
		velocity.Z = 0.0f;
		velocity *= m_Variables.bIsMovingForward ? 1.0f : -1.0f;

		const FTransform actorTransform = GetOwner()->GetActorTransform();
		yawDelta = actorTransform.InverseTransformVector(velocity).ToOrientationRotator().Yaw;
		const float newAimYaw = FMath::FInterpTo(m_Variables.AimYaw, yawDelta, DeltaTime, m_TurnSpeed);		
		m_Variables.AimYaw = MapAngleTo180(newAimYaw);;
	}

	m_ActorYawLastFrame = actorRotation.Yaw;
	m_AimYawLastFrame = m_Variables.AimYaw;

	if (!GetOwner()->HasAuthority())
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
	if (bIsTurning || m_TurnAnimationPlaying)
	{
		return Angle;
	}
	else
	{
		return MapAngleTo180_Forced(Angle);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::AddCurveValueToYawWhenTurning(float& Yaw)
{
	const bool bIsActive = m_AnimInstance->Montage_IsActive(m_TurnAnimationPlaying);
	if (bIsActive && m_TurnAnimationPlaying)
	{
		const float curveValue = m_AnimInstance->GetCurveValue("DistanceCurve");
		if (m_bCurveIsPlaying)
		{
			const float curveDelta = m_CurveValueLastFrame - curveValue;
			Yaw += curveDelta;
			m_CurveValueLastFrame = curveValue;	
		}
		else if (curveValue)
		{
			/* The first time we got a value from the curve, we want to just set the last curve value and not update delta yaw. */
			m_CurveValueLastFrame = curveValue;
			m_bCurveIsPlaying = true;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::CheckWhetherToPlayTurnAnimation(float DeltaTime, float NewAimYaw)
{
	/*const bool bIsTurning = m_AnimInstance->Montage_IsActive(m_TurnAnimationPlaying);*/
	const float aimYaw_Abs = FMath::Abs(NewAimYaw);
	if (m_TurnAnimationPlaying || aimYaw_Abs < 90.0f)
	{
		return;
	}

	const float aimYawDelta = NewAimYaw - m_AimYawLastFrame;
	const float predictedYaw = (aimYawDelta * (m_180TurnPredictionTime / DeltaTime)) + NewAimYaw;
	// Play the turn animation faster to avoid over bending the upper body.
	m_TurnAnimPlayRate = FMath::Abs(predictedYaw) > (m_180TurnThreshold * 1.5f) ? 1.75f : 1.0f;

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

	m_bCurveIsPlaying = false;
	m_AnimInstance->Montage_Play(m_TurnAnimationPlaying, m_TurnAnimPlayRate, EMontagePlayReturnType::MontageLength, 0.0f, false);

	if (m_TurnAnimationPlaying)
	{
		FString role = GetOwner()->HasAuthority() ? "Server" : "Client";
		UE_LOG(LogTemp, Display, TEXT("'%s'[%s]: Turn animation playing."), *GetOwner()->GetName(), *role);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::CheckIfTurnAnimFinished()
{
	const bool bIsActive = m_AnimInstance->Montage_IsActive(m_TurnAnimationPlaying);
	if(!bIsActive && m_TurnAnimationPlaying)
	{
		m_TurnAnimationPlaying = nullptr;
		m_bCurveIsPlaying = false;

		FString role = GetOwner()->HasAuthority() ? "Server" : "Client";
		UE_LOG(LogTemp, Warning, TEXT("'%s'[%s]: Turn animation stopped."), *GetOwner()->GetName(), *role);
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
	APawn* instigator = GetOwner()->GetInstigator();
	if(instigator->IsLocallyControlled())
	{
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
	float velocity = m_Variables.HorizontalVelocity;
	if (m_MovementComponent && !m_MovementComponent->IsMovingOnGround())
	{
		m_Variables.MovementType = EMovementType::InAir;
	}
	else if (velocity == 0.0f)
	{
		m_Variables.MovementType = EMovementType::Idle;
	}
	else
	{
		m_Variables.MovementType = velocity > m_SprintingSpeedThreshold ? EMovementType::Sprinting : EMovementType::Moving;
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
void UBasicAnimationSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UBasicAnimationSystemComponent, m_AimYaw, COND_SkipOwner);
}