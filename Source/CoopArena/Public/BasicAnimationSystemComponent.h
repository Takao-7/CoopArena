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

	FBASVariables _variables;
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Basic Animation System")
	float _YawActorLastTick;

	UPROPERTY(BlueprintReadOnly, Category = "Basic Animation System")
	float _YawActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System")
	float _MaxCrouchSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System")
	float _MaxWalkSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System")
	float _MaxJogSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System")
	float _MaxSprintSpeed;

	/* Speed, in degree per second, used to rotate the actor towards the control rotation while moving. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System")
	float _MovingTurnSpeed;

	/* Speed used for resetting the view direction. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System")
	float _ViewDirectionResetSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System")
	float _CrouchHalfHeight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System")
	float _MaxAcceleration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System")
	float _JumpZVelocity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System")
	float _AirControl;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Basic Animation System")
	float _BrakingDecleration;	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;	

public:	
	UBasicAnimationSystemComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Basic Animation System")
	FBASVariables GetActorVariables() const;

	/* Begin BAS Interface */
	bool IsCrouching_Implementation() override;
	bool IsAiming_Implementation() override;
	EWEaponType GetEquippedWeaponType_Implementation() override;
	/* End BAS Interface */

private:
	void SetYawActor();

	void SetViewDirection(float DeltaTime);

	void SetUseControlRotationYawOnCharacter();

	void SetMovementComponentValues();

	void SetAimPitch();

	void SetIsMovingForward();

	FVector GetMovementInputVectorLocalSpace();

	void SetMovementType();

	void SetHorizontalVelocity();

	void SetInputDirection();
};