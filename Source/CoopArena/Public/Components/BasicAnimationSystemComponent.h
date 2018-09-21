// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/BAS_Interface.h"
#include "BasicAnimationSystemComponent.generated.h"


class UCharacterMovementComponent;
class IBAS_Interface;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJump_Signature);


USTRUCT(BlueprintType)
struct FBASVariables
{
	GENERATED_BODY()

	/* The yaw angle between the control rotation and the velocity direction. */
	UPROPERTY(BlueprintReadOnly)
	float AimYaw;

	/* The pitch angle between the control rotation and the actor's pitch.  */
	UPROPERTY(BlueprintReadOnly)
	float AimPitch;

	/* The actor's horizontal speed. */
	UPROPERTY(BlueprintReadOnly)
	float HorizontalVelocity;

	/* Is the actor moving forward. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsMovingForward;

	/* Is the actor aiming through his weapon's ironsights. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsAiming;

	/* The actor's current movement type. */
	UPROPERTY(BlueprintReadOnly)
	EMovementType MovementType;

	UPROPERTY(BlueprintReadOnly)
	EMovementAdditive MovementAdditive;

	UPROPERTY(BlueprintReadOnly)
	EWEaponType EquippedWeaponType;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPARENA_API UBasicAnimationSystemComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	/* If the yaw value is at least this value, we will make a 180 degree and not a 90 degree turn. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System|Turning", meta = (DisplayName = "180 degree turn threshold", ClampMin = 100.0f, ClampMax = 180.0f))
	float m_180TurnThreshold;

	/**
	 * When the yaw angle (m_variables.AimYaw) is greater than 90 degree, we calculate the yaw angle this many seconds into the future. If that angle is greater than the '180 degree turn threshold'
	 * we will make a 180 degree and not a 90 degree turn.
	 */ 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System|Turning", meta = (DisplayName = "180 degree turn prediction time", ClampMin = 0.0f, ClampMax = 1.0f))
	float m_180TurnPredictionTime;

	/* When the yaw value is at least this value, it will be clamped at +/- 180 degree */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System|Turning", meta = (DisplayName = "Angle clamp threshold", ClampMin = 180.0f, ClampMax = 360.0f))
	float m_AngleClampThreshold;

	/* How fast the actor rotates towards his moving direction. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System|Turning", meta = (DisplayName = "Turn speed"))
	float m_TurnSpeed;

	UPROPERTY(EditAnywhere, Category = "Basic Animation System|Turning", meta = (DisplayName = "Turn left 90 degree animation"))
	UAnimMontage* m_TurnLeft90Animation;

	UPROPERTY(EditAnywhere, Category = "Basic Animation System|Turning", meta = (DisplayName = "Turn right 90 degree animation"))
	UAnimMontage* m_TurnRight90Animation;

	UPROPERTY(EditAnywhere, Category = "Basic Animation System|Turning", meta = (DisplayName = "Turn left 180 degree animation"))
	UAnimMontage* m_TurnLeft180Animation;

	UPROPERTY(EditAnywhere, Category = "Basic Animation System|Turning", meta = (DisplayName = "Turn right 180 degree animation"))
	UAnimMontage* m_TurnRight180Animation;

private:
	/* If the character's speed is greater than this, he is sprinting. */
	float m_SprintingSpeedThreshold;

	/* The owner's character movement component. */
	UCharacterMovementComponent* m_MovementComponent;

	/* Relevant calculated variables from the actor. */
	FBASVariables m_Variables;

	float m_ActorYawLastFrame;
	float m_CurveValueLastFrame;
	float m_AimYawLastFrame;
	bool m_bCurveIsPlaying;
	bool m_bIsTurningRight;

	float m_ActorYawOnStop;

	/* The turn animation that is currently playing or nullptr if there is no turn animation playing. */
	UAnimMontage* m_TurnAnimationPlaying;

	/* Owner's animation instance. */
	UAnimInstance* m_AnimInstance;

protected:
	virtual void BeginPlay() override;

public:
	/**
	* @param SprintingSpeedThreshold When the actor is moving faster than this value, then he is sprinting.
	*/
	void SetSprintingSpeedThreshold(float SprintingSpeedThreshold);

	/* Event to call when the actor wants to jump. */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Basic Animation System", meta = (DisplayName = "On jump event"))
	FOnJump_Signature OnJumpEvent;

	UBasicAnimationSystemComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	

	UFUNCTION(BlueprintPure, Category = "Basic Animation System")
	FBASVariables& GetActorVariables();

private:
	void FindAnimInstance();
	void FindCharacterMovementComponent();
	void CheckIfLocallyControlled();
	void CheckIfTurnAnimFinished();
	void ClampAimYaw();
	void ClampYawDelta(float& YawDelta);

	void SetMovementType();
	void SetMovmentAdditive();
	void SetHorizontalVelocity();
	void SetIsMovingForward();
	void SetAimYaw(float DeltaTime);
	void SetAimPitch();

	/* Adds the curve delta value (value last frame - value this frame) to the given deltaYaw if a turn animation is playing. */
	void AddCurveValueToDeltaWhenTurning(float& deltaYaw);

	/* Checks if the absolute yaw angle is at least 90� and if so starts playing the turn animation montage. */
	void CheckWhetherToPlayTurnAnimation(float DeltaTime);

	UFUNCTION(Server, WithValidation, Unreliable)
	void SetAimPitch_Server(float AimPitch);

	UFUNCTION(NetMulticast, Unreliable)
	void SetAimPitch_Multicast(float AimPitch);

	UFUNCTION(Server, WithValidation, Unreliable)
	void SetAimYaw_Server(float AimYaw);

	UFUNCTION(NetMulticast, Unreliable)
	void SetAimYaw_Multicast(float AimYaw);
};