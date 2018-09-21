#include "BasicAnimationSystemComponent.h"
#include "Interfaces/BAS_Interface.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerCharacter.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"
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

	m_180TurnThreshold = 165.0f;
	m_AngleClampThreshold = 225.0f;
	m_180TurnPredictionTime = 0.2f;
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	FindAnimInstance();
	FindCharacterMovementComponent();
	
	m_ActorYawLastFrame = GetOwner()->GetActorRotation().Yaw;
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
void UBasicAnimationSystemComponent::FindAnimInstance()
{
	ACharacter* character = Cast<ACharacter>(GetOwner());
	if (character)
	{
		m_AnimInstance = character->GetMesh()->GetAnimInstance();
	}
	else
	{
		UE_LOG(LogTemp, Fatal, TEXT("'%s' does not have an animation instance!"), *GetOwner()->GetName());
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
	SetAimYaw(DeltaTime);
	SetAimPitch();
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetHorizontalVelocity()
{
	FVector velocity = GetOwner()->GetVelocity();
	velocity.Z = 0.0f;
	float horizontalVelocity = velocity.Size();

	FTransform actorTransform = GetOwner()->GetActorTransform();
	float yawDelta = actorTransform.InverseTransformVector(velocity).ToOrientationRotator().Yaw;
	horizontalVelocity *= FMath::Abs(yawDelta) > 120.0f ? -1.0f : 1.0f;

	horizontalVelocity = FMath::IsNearlyZero(horizontalVelocity, 0.01f) ? 0.0f : horizontalVelocity;
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
void UBasicAnimationSystemComponent::SetAimYaw(float DeltaTime)
{
	FRotator actorRotation = GetOwner()->GetActorRotation();
	m_AimYawLastFrame = m_Variables.AimYaw;

	float yawDelta = 0.0f;
	if (m_Variables.MovementType == EMovementType::Idle)
	{
		AddCurveValueToDeltaWhenTurning(yawDelta);
		yawDelta += m_ActorYawLastFrame - actorRotation.Yaw;
		ClampYawDelta(yawDelta);
		m_Variables.AimYaw += yawDelta;

		CheckWhetherToPlayTurnAnimation(DeltaTime);
	}
	else
	{
		FVector velocity = GetOwner()->GetVelocity();
		velocity.Z = 0.0f;
		velocity *= m_Variables.bIsMovingForward ? 1.0f : -1.0f;

		FTransform actorTransform = GetOwner()->GetActorTransform();
		yawDelta = actorTransform.InverseTransformVector(velocity).ToOrientationRotator().Yaw;
		m_Variables.AimYaw = FMath::FInterpTo(m_Variables.AimYaw, yawDelta, DeltaTime, m_TurnSpeed);
		ClampAimYaw();
	}

	m_ActorYawLastFrame = actorRotation.Yaw;
	
	CheckIfTurnAnimFinished();
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::ClampYawDelta(float& YawDelta)
{
	while (YawDelta > m_AngleClampThreshold)
	{
		UE_LOG(LogTemp, Warning, TEXT("Clamping yaw delta %f"), YawDelta);
		YawDelta -= 360.0f;
	}

	while (YawDelta < -m_AngleClampThreshold)
	{
		UE_LOG(LogTemp, Warning, TEXT("Clamping yaw delta %f"), YawDelta);
		YawDelta += 360.0f;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::AddCurveValueToDeltaWhenTurning(float& deltaYaw)
{
	const bool bIsTurning = m_AnimInstance->GetCurveValue("IsTurning");
	if (bIsTurning)
	{
		float curveValue;
		const bool bFoundValue = m_AnimInstance->GetCurveValue("DistanceCurve", curveValue);
		if (m_bCurveIsPlaying)
		{
			const float curveDelta = m_CurveValueLastFrame - curveValue;
			deltaYaw += curveDelta;
			m_CurveValueLastFrame = curveValue;
		}
		else if (bFoundValue)
		{
			/* The first time we got a value from the curve, we want to just set the last curve value and not update delta yaw. */
			m_CurveValueLastFrame = curveValue;
			m_bCurveIsPlaying = true;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::CheckWhetherToPlayTurnAnimation(float DeltaTime)
{
	const bool bIsTurning = m_AnimInstance->GetCurveValue("IsTurning");
	const float aimYaw_Abs = FMath::Abs(m_Variables.AimYaw);
	if (bIsTurning || aimYaw_Abs < 90.0f)
	{
		return;
	}

	const float aimYawDelta = m_Variables.AimYaw - m_AimYawLastFrame;
	const float predictedYaw = (aimYawDelta * (m_180TurnPredictionTime / DeltaTime)) + m_Variables.AimYaw;

	if (m_Variables.AimYaw <= -90.0f) // Turn right
	{
		if (predictedYaw <= -m_180TurnThreshold && m_TurnRight180Animation)
		{
			m_TurnAnimationPlaying = m_TurnRight180Animation;
		}
		else
		{
			m_TurnAnimationPlaying = m_TurnRight90Animation;
		}		
		m_bIsTurningRight = true;		
	}
	else  // Turn left
	{
		if (predictedYaw >= m_180TurnThreshold && m_TurnLeft180Animation)
		{
			m_TurnAnimationPlaying = m_TurnLeft180Animation;
		}
		else
		{
			m_TurnAnimationPlaying = m_TurnLeft90Animation;
		}
		m_bIsTurningRight = false;
	}

	m_bCurveIsPlaying = false;
	m_AnimInstance->Montage_Play(m_TurnAnimationPlaying, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, false);
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetAimYaw_Server_Implementation(float AimYaw)
{
	SetAimYaw_Multicast(AimYaw);
}

bool UBasicAnimationSystemComponent::SetAimYaw_Server_Validate(float AimYaw)
{
	return true;
}

void UBasicAnimationSystemComponent::SetAimYaw_Multicast_Implementation(float AimYaw)
{
	m_Variables.AimYaw = AimYaw;
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::CheckIfTurnAnimFinished()
{
	if (m_TurnAnimationPlaying)
	{
		const bool bIsTurning = m_AnimInstance->GetCurveValue("IsTurning");
		if (!bIsTurning)
		{
			m_TurnAnimationPlaying = nullptr;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::ClampAimYaw()
{
	while (m_Variables.AimYaw > m_AngleClampThreshold)
	{
		UE_LOG(LogTemp, Warning, TEXT("Clamping angle %f"), m_Variables.AimYaw);
		m_Variables.AimYaw -= 360.0f;
	}

	while (m_Variables.AimYaw < -m_AngleClampThreshold)
	{
		UE_LOG(LogTemp, Warning, TEXT("Clamping angle %f"), m_Variables.AimYaw);
		m_Variables.AimYaw += 360.0f;
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

/////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetMovementType()
{
	float velocity = m_Variables.HorizontalVelocity;
	if (velocity == 0.0f)
	{
		m_Variables.MovementType = EMovementType::Idle;
	}
	else if (velocity > m_SprintingSpeedThreshold)
	{
		m_Variables.MovementType = EMovementType::Sprinting;
	}
	else if (m_MovementComponent && !m_MovementComponent->IsMovingOnGround())
	{
		m_Variables.MovementType = EMovementType::Jumping;
	}
	else
	{
		m_Variables.MovementType = EMovementType::Moving;
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
//void UBasicAnimationSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//	//DOREPLIFETIME_CONDITION(UBasicAnimationSystemComponent, m_Pitch, COND_SkipOwner);
//	//DOREPLIFETIME_CONDITION(UBasicAnimationSystemComponent, m_Yaw, COND_SkipOwner);
//}