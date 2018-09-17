// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/BAS_Interface.h"
#include "BasicAnimationSystemComponent.generated.h"


class UCharacterMovementComponent;
class IBAS_Interface;


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
	/* The angle between the actor's feed and his head (controller) at which the actor will turn. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System", meta = (DisplayName = "Idle turn angle threshold"))
	float m_IdleTurnAngleThreshold;

	/* The actor's max speed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System", meta = (DisplayName = "Max sprint speed"))
	float m_MaxActorSpeed;

	/* How fast the actor rotates towards his moving direction. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System", meta = (DisplayName = "Turn speed"))
	float m_TurnSpeed;

private:
	/* The owner's character movement component. */
	UCharacterMovementComponent* _MovementComponent;

	/* Relevant calculated variables from the actor. */
	UPROPERTY(Replicated)
	FBASVariables m_Variables;

	float m_AimYawLastFrame;

protected:
	virtual void BeginPlay() override;

public:	
	UBasicAnimationSystemComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	

	UFUNCTION(BlueprintPure, Category = "Basic Animation System")
	FBASVariables GetActorVariables() const;

private:
	void SetHorizontalVelocity();
	void SetIsMovingForward();
	void SetMovementType();
	void SetAimYaw(float DeltaTime);
	void SetAimPitch();

	UFUNCTION(Server, Unreliable, WithValidation)
	void SetVariables_Server(FBASVariables Variables);
};