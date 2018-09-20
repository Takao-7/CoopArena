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
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	FindAnimInstance();
	FindCharacterMovementComponent();
	
	m_YawLastFrame = GetOwner()->GetActorRotation().Yaw;
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
	//SetMovmentAdditive();
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
	if (m_Variables.MovementType == EMovementType::Idle)
	{
		CheckWhetherToPlayTurnAnimation();

		float deltaYaw = 0.0f;
		AddCurveValueToDeltaWhenTurning(deltaYaw);
		deltaYaw += m_YawLastFrame - actorRotation.Yaw;

		m_Variables.AimYaw += deltaYaw;
	}
	else
	{
		FVector velocity = GetOwner()->GetVelocity();
		velocity.Z = 0.0f;
		velocity *= m_Variables.bIsMovingForward ? 1.0f : -1.0f;

		FTransform actorTransform = GetOwner()->GetActorTransform();
		float yawDelta = actorTransform.InverseTransformVector(velocity).ToOrientationRotator().Yaw;
		m_Variables.AimYaw = FMath::FInterpTo(m_Variables.AimYaw, yawDelta, DeltaTime, m_TurnSpeed);
	}
	m_YawLastFrame = actorRotation.Yaw;
	ClampAimYaw();	
	if (FMath::Abs(m_Variables.AimYaw) > 90.0f)
	{
		UE_LOG(LogTemp, Display, TEXT("Aim yaw: %f"), m_Variables.AimYaw);
	}
	CheckIfTurnAnimFinished();
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
			//UE_LOG(LogTemp, Display, TEXT("Curve delta: %f"), curveDelta);
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
void UBasicAnimationSystemComponent::CheckWhetherToPlayTurnAnimation()
{
	if (m_Variables.AimYaw <= -90.0f && m_TurnAnimationPlaying == nullptr && m_TurnRight90Animation)
	{
		UE_LOG(LogTemp, Warning, TEXT("Aim yaw: %f"), m_Variables.AimYaw);
		m_TurnAnimationPlaying = m_TurnRight90Animation;
		m_bCurveIsPlaying = false;
		m_bIsTurningRight = true;
		m_AnimInstance->Montage_Play(m_TurnAnimationPlaying, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, false);
		//UE_LOG(LogTemp, Warning, TEXT("Turning right"));
	}
	else if (m_Variables.AimYaw >= 90.0f && m_TurnAnimationPlaying == nullptr && m_TurnLeft90Animation)
	{
		UE_LOG(LogTemp, Warning, TEXT("Aim yaw: %f"), m_Variables.AimYaw);
		m_TurnAnimationPlaying = m_TurnLeft90Animation;
		m_bCurveIsPlaying = false;
		m_bIsTurningRight = false;
		m_AnimInstance->Montage_Play(m_TurnAnimationPlaying, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, false);
		//UE_LOG(LogTemp, Warning, TEXT("Turning left"));
	}
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
		bool bMontageIsFinished = !m_AnimInstance->Montage_IsPlaying(m_TurnAnimationPlaying);
		if (bMontageIsFinished)
		{
			m_TurnAnimationPlaying = nullptr;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::ClampAimYaw()
{
	if (m_Variables.AimYaw > 180.0f)
	{
		m_Variables.AimYaw -= 360.0f;
	}
	else if (m_Variables.AimYaw < -180.0f)
	{
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