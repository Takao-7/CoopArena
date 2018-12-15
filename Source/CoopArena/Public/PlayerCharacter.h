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

public:
	APlayerCharacter();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Tick(float DeltaTime) override;

	/* Returns the characters camera location in world space. If there is no camera, then
	 * this function returns a ZeroVector */
	UFUNCTION(BlueprintPure, Category = "PlayerCharacter")
	FVector GetCameraLocation() const;

	UFUNCTION(BlueprintPure, Category = "PlayerCharacter")
	UCameraComponent* GetActiveCamera() const;

	UFUNCTION(BlueprintPure, Category = "PlayerCharacter")
	UCameraComponent* GetFirstPersonCamera() { return _FirstPersonCamera; };

	void SetThirdPersonCameraToActive();

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter")
	UCameraComponent* _FirstPersonCamera;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter")
	UCameraComponent* _ThirdPersonCamera;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter")
	USpringArmComponent* _SpringArm;

	UPROPERTY(BlueprintReadWrite, Category = "PlayerCharacter")
	UCameraComponent* _LastCamera;

	virtual void OnHolsterWeapon() override;

	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleOnDestroy(AActor* DestroyedActor);

	UFUNCTION()
	void HandleOnDeath(AActor* DeadActor, AController* ActorController, AController* Killer);


	/////////////////////////////////////////////////////
					/* Movement */
	/////////////////////////////////////////////////////
protected:
	/** Base turn rate, in deg/sec */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter", meta = (DisplayName = "Base turn rate"))
	float _BaseTurnRate;

	/** Base look up/down rate, in deg/sec */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter", meta = (DisplayName = "Base Look up rate"))
	float _BaseLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter", meta = (DisplayName = "Toggle prone"))
	bool _bToggleProne;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter", meta = (DisplayName = "Toggle sprinting"))
	bool _bToggleSprinting;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter", meta = (DisplayName = "Toggle crouching"))
	bool _bToggleCrouching;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter", meta = (DisplayName = "Toggle crouching"))
	bool _bToggleWalking;

	/**
	* Called via input to turn at a given rate.
	* @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void TurnAtRate(float value);

	/**
	* Called via input to turn look up/down at a given rate.
	* @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void LookUpAtRate(float value);


	/////////////////////////////////////////////////////
						/* Interaction */
	/////////////////////////////////////////////////////
protected:
	UFUNCTION(BlueprintCallable, Category = "PlayerCharacter")
	void OnBeginInteracting();

	UFUNCTION(BlueprintCallable, Category = "PlayerCharacter")
	void OnEndInteracting();

	UFUNCTION(BlueprintCallable, Category = "PlayerCharacter")
	void CheckForInteractables();

	/**
	 * Sets the actor in focus, both as AActor* (_ActorInFocus) and IInteractable* (_InteractableInFocus).
	 * @param actor The actor that should be set. If it's not implementing the IInteractable interface,
	 * both will be set to nullptr.
	 * If actor is nullptr, both will be set as nullptr, too.
	 */
	UFUNCTION(BlueprintCallable, Category = "PlayerCharacter")
	void SetActorInFocus(AActor* actor);

	UFUNCTION(BlueprintCallable, Category = "PlayerCharacter")
	void SetComponentInFocus(UPrimitiveComponent* Component);

	/* The distance, in cm, at which the character can interact with interactables */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PlayerCharacter")
	float _InteractionRange;

	/* The hit result for the line trace that runs every frame to look for interactables */
	UPROPERTY(BlueprintReadOnly, Category = "PlayerCharacter")
	FHitResult _InteractionHitResult;

	/* The intractable actor that is currently in focus (=the actor that this character is aiming at) */
	IInteractable* _InteractableInFocus;
	AActor* _ActorInFocus;
	UPrimitiveComponent* _ComponentInFocus;

public:
	UFUNCTION(BlueprintCallable, Category = "PlayerCharacter")
	bool InteractionLineTrace(FHitResult& outHitresult);

	UFUNCTION(BlueprintCallable, Category = "PlayerCharacter")
	const FHitResult& GetInteractionLineTraceHitResult() const;


	/////////////////////////////////////////////////////
						/* Input */
	/////////////////////////////////////////////////////
protected:
	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void OnPronePressed();

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void OnProneReleased();

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void OnWalkPressed();

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void OnWalkReleased();

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void OnSprintPressed();

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void OnSprintReleased();

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void OnCrouchPressed();

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void OnCrouchReleased();

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void OnChangeCameraPressed();

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void OnAimingPressed();

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void OnAimingReleased();

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void OnSelectPrimaryWeapon();

	UFUNCTION(BlueprintCallable, Category = Humanoid)
	void OnSelectSecondaryWeapon();


	/////////////////////////////////////////////////////
						/* Networking */
	/////////////////////////////////////////////////////
private:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_OnBeginInteracting(AActor* ActorInFocus, UPrimitiveComponent* ComponentInFocus);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_OnEndInteracting(AActor* ActorInFocus);

};