// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enums/BAS_Enums.h"
#include "Enums/WeaponEnums.h"
#include "AdequateAnimationSystem.generated.h"


USTRUCT(BlueprintType)
struct FAASVariables
{
	GENERATED_BODY()

	/**
	* The yaw angle between the control rotation and the input direction. When the actor is idle, this is the last input direction before stopping.
	* This means, that this value is not being updated while the actor is idle (= not moving).
	*/
	UPROPERTY(BlueprintReadOnly)
	float InputDirection;

	/* The input direction in the last frame. This value is not being updated while the actor is idle (= not moving). */
	UPROPERTY(BlueprintReadOnly)
	float LastInputDirection;

	/* The pitch angle between the control rotation and the actor's pitch.  */
	UPROPERTY(BlueprintReadOnly)
	float AimPitch;

	/* The actor's horizontal speed. */
	UPROPERTY(BlueprintReadOnly)
	float HorizontalVelocity;

	/* Is the actor moving forward. */
	UPROPERTY(BlueprintReadOnly)
	bool bIsMovingForward;

	/* Was the actor moving forward in the last frame. */
	UPROPERTY(BlueprintReadOnly)
	bool bWasMovingForward;

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
class COOPARENA_API UAdequateAnimationSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAdequateAnimationSystem();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
	
};
