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

	m_IdleTurnAngleThreshold = 90.0f;
	m_MaxActorSpeed = 600.0f;
	m_TurnSpeed = 10.0f;

	m_Variables.MovementType = EMovementType::Idle;
	m_Variables.MovementAdditive = EMovementAdditive::None;
	m_Variables.EquippedWeaponType = EWEaponType::None;

	bReplicates = true;
	bAutoActivate = true;

	m_bTurnAnimIsPlaying = false;
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

	bool bIsLocallyControlled = GetOwner()->GetInstigator()->IsLocallyControlled();
	if (!bIsLocallyControlled)
	{
		PrimaryComponentTick.SetTickFunctionEnable(false);
	}

	m_AimYawLastFrame = GetOwner()->GetActorRotation().Yaw;

	/* Get animation instance */
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
void UBasicAnimationSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetHorizontalVelocity();
	SetIsMovingForward();
	SetMovementType();
	SetAimYaw(DeltaTime);
	SetAimPitch();
	m_Variables.EquippedWeaponType = IBAS_Interface::Execute_GetEquippedWeaponType(GetOwner());

	SetVariables_Server(m_Variables);
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetHorizontalVelocity()
{
	FVector velocity = GetOwner()->GetVelocity();
	velocity.Z = 0.0f;
	float horizontalVelocity = velocity.Size();
	horizontalVelocity = FMath::IsNearlyZero(horizontalVelocity, 0.01f) ? 0.0f : horizontalVelocity;

	FTransform actorTransform = GetOwner()->GetActorTransform();
	float yawDelta = actorTransform.InverseTransformVector(velocity).ToOrientationRotator().Yaw;
	horizontalVelocity *= FMath::Abs(yawDelta) > 120.0f ? -1.0f : 1.0f;

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
void UBasicAnimationSystemComponent::SetMovementType()
{
	m_Variables.MovementType = IBAS_Interface::Execute_GetMovementType(GetOwner());
	//m_Variables.MovementAdditive = IBAS_Interface::Execute_GetMovementAdditive(GetOwner());
}


//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetAimYaw(float DeltaTime)
{
	FRotator actorRotation = GetOwner()->GetActorRotation();

	if (m_Variables.HorizontalVelocity == 0.0f)
	{
		if (m_Variables.AimYaw <= -90.0f && !m_bTurnAnimIsPlaying && TurnRightAnimation)
		{
			m_AnimInstance->Montage_Play(TurnRightAnimation);
			m_bTurnAnimIsPlaying = true;
			m_bIsTurningRight = true;
		}
		else if (m_Variables.AimYaw >= 90.0f && !m_bTurnAnimIsPlaying && TurnLeftAnimation)
		{
			m_AnimInstance->Montage_Play(TurnLeftAnimation);
			m_bTurnAnimIsPlaying = true;
			m_bIsTurningRight = false;
		}		

		if (m_bTurnAnimIsPlaying)
		{
			float curveValue;
			bool bFoundCurveValue = m_AnimInstance->GetCurveValue("DistanceCurve", curveValue);
			if(bFoundCurveValue)
			{
				float deltaYaw = 90.0f + curveValue;
				m_Variables.AimYaw += deltaYaw * m_bIsTurningRight ? 1.0f : -1.0f;
			}
		}
		else
		{
			float deltaYaw = m_AimYawLastFrame - actorRotation.Yaw;
			m_Variables.AimYaw += deltaYaw;
		}
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

	m_AimYawLastFrame = actorRotation.Yaw;
	ClampAimYaw();
	
	if (m_bTurnAnimIsPlaying)
	{
		bool bLeftAnimPlaying = m_AnimInstance->Montage_IsPlaying(TurnLeftAnimation);
		bool bRightAnimPlaying = m_AnimInstance->Montage_IsPlaying(TurnRightAnimation);
		if (!bLeftAnimPlaying && !bRightAnimPlaying)
		{
			m_bTurnAnimIsPlaying = false;
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
	float controlPitch = GetOwner()->GetInstigator()->GetControlRotation().Pitch;
	if (controlPitch >= 90.0f)
	{
		controlPitch -= 360.0f;
	}
	m_Variables.AimPitch = controlPitch;
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::SetVariables_Server_Implementation(FBASVariables Variables)
{
	m_Variables = Variables;
}

bool UBasicAnimationSystemComponent::SetVariables_Server_Validate(FBASVariables Variables)
{
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////
FBASVariables& UBasicAnimationSystemComponent::GetActorVariables()
{
	return m_Variables;
}

//////////////////////////////////////////////////////////////////////////////////////
void UBasicAnimationSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBasicAnimationSystemComponent, m_Variables);
}