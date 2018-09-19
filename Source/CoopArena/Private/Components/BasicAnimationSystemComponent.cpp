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
void UBasicAnimationSystemComponent::OnRepAimPitch()
{
	m_Variables.AimPitch = m_AimPitch;
}

void UBasicAnimationSystemComponent::OnRepYaw()
{
	m_Variables.AimYaw = m_Yaw;
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
	SetAimYaw(DeltaTime);
	SetAimPitch();
	SetMovementType();
	SetMovmentAdditive();
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

	if (m_Variables.HorizontalVelocity == 0.0f)
	{
		CheckWhetherToPlayTurnAnimation();

		float deltaYaw = m_YawLastFrame - actorRotation.Yaw;
		AddCurveValueToDeltaWhenTurning(deltaYaw);

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
	
	CheckIfTurnAnimFinished();
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::AddCurveValueToDeltaWhenTurning(float& deltaYaw)
{
	if (m_TurnAnimationPlaying)
	{
		float curveValue;
		m_AnimInstance->GetCurveValue("DistanceCurve", curveValue);
		float curveDelta = m_CurveValueLastFrame - curveValue;
		deltaYaw += curveValue * m_bIsTurningRight ? 1.0f : -1.0f;
		m_CurveValueLastFrame = curveValue;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::CheckWhetherToPlayTurnAnimation()
{
	if (m_Variables.AimYaw <= -90.0f && m_TurnAnimationPlaying == nullptr && m_TurnRight90Animation)
	{
		m_TurnAnimationPlaying = m_TurnRight90Animation;
		m_AnimInstance->Montage_Play(m_TurnAnimationPlaying);
		m_AnimInstance->GetCurveValue("DistanceCurve", m_CurveValueLastFrame);
		m_bIsTurningRight = true;
	}
	else if (m_Variables.AimYaw >= 90.0f && m_TurnAnimationPlaying == nullptr && m_TurnLeft90Animation)
	{
		m_TurnAnimationPlaying = m_TurnLeft90Animation;
		m_AnimInstance->Montage_Play(m_TurnAnimationPlaying);
		m_AnimInstance->GetCurveValue("DistanceCurve", m_CurveValueLastFrame);
		m_bIsTurningRight = false;
	}
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
	if(GetOwner()->GetInstigator()->IsLocallyControlled())
	{
		float controlPitch = GetOwner()->GetInstigator()->GetControlRotation().Pitch;
		if (controlPitch >= 90.0f)
		{
			controlPitch -= 360.0f;
		}
		m_Variables.AimPitch = controlPitch;
		Server_SetAimPitch(controlPitch);
	}
}

void UBasicAnimationSystemComponent::Server_SetAimPitch_Implementation(float AimPitch)
{
	m_AimPitch = AimPitch;
}

bool UBasicAnimationSystemComponent::Server_SetAimPitch_Validate(float AimPitch)
{
	return true;
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
void UBasicAnimationSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UBasicAnimationSystemComponent, m_AimPitch, COND_SkipOwner);
	//DOREPLIFETIME_CONDITION(UBasicAnimationSystemComponent, m_Yaw, COND_SkipOwner);
}