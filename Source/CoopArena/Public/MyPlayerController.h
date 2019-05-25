// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

class ADefaultHUD;

/**
 * 
 */
UCLASS()
class COOPARENA_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Player Controller", meta = (DisplayName = "Class to respawn as"))
	TSubclassOf<APlayerCharacter> _ClassToRespawnAs;

	virtual void BeginPlay() override;

public:
	virtual void OnPossess(APawn* aPawn) override;

	void StartSpectating(AActor* ActorToSpectate = nullptr);

	void StopSpectating();

	UFUNCTION(Client, Reliable)
	void StartSpectating_Client(APlayerCharacter* PlayerToSpectate = nullptr);

	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintPure, Category = "Player Controller")
	bool IsSpectating() const;

	const FVector& GetDeathLocation() const;
	APlayerCharacter* GetLastPossessedCharacter();

	virtual void ClientTeamMessage_Implementation(class APlayerState* SenderPlayerState, const FString& S, FName Type, float MsgLifeTime = 0) override;	

	ADefaultHUD* GetDefaultHUD() const;

	FString PlayerNameTemp;

private:
	UFUNCTION(Server, WithValidation, Reliable)
	void SpectateNextPlayer_Server();

	UFUNCTION(Server, WithValidation, Reliable)
	void SpectatePreviousPlayer_Server();

	void OnOpenMenuPressed();
	void OnOpenScoreBoard();

	FVector _DeathLocation;	
	APlayerCharacter* _MyCharacter;

};
