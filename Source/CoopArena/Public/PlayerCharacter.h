// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Humanoid.h"
#include "PlayerCharacter.generated.h"


class UCameraComponent;
class USpringArmComponent;
class IInteractable;


UCLASS()
class COOPARENA_API APlayerCharacter : public AHumanoid
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = PlayerCharacter)
	UCameraComponent* _FirstPersonCamera;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = PlayerCharacter)
	UCameraComponent* _ThirdPersonCamera;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = PlayerCharacter)
	USpringArmComponent* _SpringArm;

	/* The distance, in cm, at which the character can interact with Interactables */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = PlayerCharacter)
	float _InteractionRange;

	/* The hit result for the line trace that runs every frame to look for Interactables */
	UPROPERTY(BlueprintReadOnly, Category = PlayerCharacter)
	FHitResult _InteractionHitResult;

	/* The interactable actor that is currently in focus (=the actor that this character is aiming at) */
	IInteractable* _InteractableInFocus;
	AActor* _ActorInFocus;
	
public:
	APlayerCharacter();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Tick(float DeltaTime) override;

	/* Returns the characters camera location in world space. If there is no camera, then 
	 * this function returns a ZeroVector */
	UFUNCTION(BlueprintPure, Category = PlayerCharacter)
	FVector GetCameraLocation() const;

	UFUNCTION(BlueprintPure, Category = PlayerCharacter)
	UCameraComponent* GetActiveCamera() const;

	UFUNCTION(BlueprintCallable, Category = PlayerCharacter)
	bool InteractionLineTrace(FHitResult& outHitresult);

	UFUNCTION(BlueprintCallable, Category = PlayerCharacter)
	const FHitResult& GetInteractionLineTraceHitResult() const;

protected:
	UFUNCTION(BlueprintCallable, Category = PlayerCharacter)
	virtual void OnBeginInteracting();

	UFUNCTION(BlueprintCallable, Category = PlayerCharacter)
	virtual void OnEndInteracting();
};
