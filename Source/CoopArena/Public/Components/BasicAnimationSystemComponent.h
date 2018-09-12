// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Structs/BASVariables.h"
#include "Interfaces/BAS_Interface.h"
#include "BasicAnimationSystemComponent.generated.h"


class UCharacterMovementComponent;
class IBAS_Interface;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPARENA_API UBasicAnimationSystemComponent : public UActorComponent, public IBAS_Interface
{
	GENERATED_BODY()

private:
	/* The owner's character movement component. */
	UCharacterMovementComponent* _MovementComponent;

	/* Relevant calculated variables from the actor. */
	UPROPERTY(Replicated)
	FBASVariables _variables;

	void RotateCharacterToMovement(float DeltaTime);
	FVector GetVelocityVectorControllerSpace();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System")
	float _IdleTurnAngleThreshold;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System")
	float _MaxSprintSpeed;

	/* Speed used to rotate the actor towards the control rotation while moving. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System")
	float _MovingTurnSpeed;

	FVector _localVelocityVector;

	FRotator _newRotationLastFrame;

	virtual void BeginPlay() override;

public:	
	UBasicAnimationSystemComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	

	UFUNCTION(BlueprintPure, Category = "Basic Animation System")
	FBASVariables GetActorVariables() const;


	/////////////////////////////////////////////////////
				/* Begin BAS Interface */
	/////////////////////////////////////////////////////
public:
	bool IsAiming_Implementation() override;
	EWEaponType GetEquippedWeaponType_Implementation() override;
	EMovementType GetMovementType_Implementation() override;
	EMovementAdditive GetMovementAdditive_Implementation() override;

	/////////////////////////////////////////////////////
				/* End BAS Interface */
	/////////////////////////////////////////////////////


private:
	void SetMovementDirection();

	/**
	 * Sets the horizontal velocity in _variables.
	 * @return The velocity vector. Z-value set to 0.
	 */ 
	FVector SetHorizontalVelocity();
	void SetMovementType();
	void SetIsMovingForward();
	void SetAimPitch();
	void SetUseControlRotationYawOnCharacter();
	void SetMovementComponentValues();
	FVector GetVelocityVectorLocalSpace();

	UFUNCTION(Server, Unreliable, WithValidation)
	void SetVariables_Server(FBASVariables Variables);
};