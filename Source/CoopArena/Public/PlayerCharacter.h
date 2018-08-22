// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Humanoid.h"
#include "PlayerCharacter.generated.h"


class UCameraComponent;
class USpringArmComponent;
class IInteractable;
class UPrimitiveComponent;


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

	/* The distance, in cm, at which the character can interact with intractables */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = PlayerCharacter)
	float _InteractionRange;

	/* The hit result for the line trace that runs every frame to look for intractables */
	UPROPERTY(BlueprintReadOnly, Category = PlayerCharacter)
	FHitResult _InteractionHitResult;

	/* The intractable actor that is currently in focus (=the actor that this character is aiming at) */
	IInteractable* _InteractableInFocus;
	AActor* _ActorInFocus;
	UPrimitiveComponent* _ComponentInFocus;

	UPROPERTY(BlueprintReadWrite, Category = PlayerCharacter)
	UCameraComponent* _LastCamera;
	
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
	virtual void ToggleAiming() override;

	UFUNCTION(BlueprintCallable, Category = PlayerCharacter)
	virtual void OnBeginInteracting();

	UFUNCTION(BlueprintCallable, Category = PlayerCharacter)
	virtual void OnEndInteracting();

	UFUNCTION(BlueprintCallable, Category = PlayerCharacter)
	void CheckForInteractables();

	virtual void OnEquipWeapon() override;

	/**
	 * Sets the actor in focus, both as AActor* (_ActorInFocus) and IInteractable* (_InteractableInFocus).
	 * @param actor The actor that should be set. If it's not implementing the IInteractable interface,
	 * both will be set to nullptr.
	 * If actor is nullptr, both will be set as nullptr, too.
	 */
	UFUNCTION(BlueprintCallable, Category = PlayerCharacter)
	void SetActorInFocus(AActor* actor);

	UFUNCTION(BlueprintCallable, Category = PlayerCharacter)
	void SetComponentInFocus(UPrimitiveComponent* Component);
};
