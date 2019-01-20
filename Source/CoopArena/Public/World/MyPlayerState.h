// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerChangedReadyStatus_Signature, APlayerState*, PlayerState, bool, bReadyStatus);


UCLASS()
class COOPARENA_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AMyPlayerState(const FObjectInitializer& ObjectInitializer);

	virtual void Reset() override;

	void SetTeamNumber(int32 NewTeamNumber);

	UFUNCTION(BlueprintPure, Category = "Player state")
	int32 GetTeamNumber() const;

	void AddKill();

	UFUNCTION(BlueprintPure, Category = "Player state")
	int32 GetKills() const;

	void AddDeath();

	UFUNCTION(BlueprintPure, Category = "Player state")
	int32 GetDeaths() const;

	void ScorePoints(int32 Points, bool bAddKill = true);

	UFUNCTION(BlueprintPure, Category = "Player state")
	float GetScore() const;

	UFUNCTION(BlueprintPure, Category = "Player state")
	bool IsAlive() const;

	UFUNCTION(BlueprintPure, Category = "Player state")
	bool IsReady() const;

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Player state", meta = (DisplayName = "Set ready status"))
	void SetReadyStatus_Server(bool bReady);

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Player state", meta = (DisplayName = "Toggle ready status"))
	void ToggleReadyStatus_Server();

	UPROPERTY(BlueprintAssignable, Category = "Player state")
	FOnPlayerChangedReadyStatus_Signature OnPlayerChangedReadyStatus;

	/* Requests the user name from the client. Will call SetPlayerName_Server */
	UFUNCTION(Client, Reliable)
	void RequestPlayerName_Client();

	UFUNCTION(BlueprintPure, Category = "Game state")
	FString GetCustomPlayerName() const { return _PlayerName; };

	virtual FString GetPlayerNameCustom() const override;

private:
	UFUNCTION()
	void OnReadyStatusReplicated();

	UFUNCTION(Server, Reliable, WithValidation)
	void SetPlayerName_Server(const FString& NewPlayerName);

	UPROPERTY(Transient, Replicated)
	int32 _NumKills;

	UPROPERTY(Transient, Replicated)
	int32 _NumDeaths;

	/* Team number assigned to player */
	UPROPERTY(Transient, Replicated)
	int32 _TeamNumber;

	UPROPERTY(Transient, Replicated)
	bool _bIsAlive;

	/* Is the player ready to start the match? */
	UPROPERTY(Transient, ReplicatedUsing=OnReadyStatusReplicated)
	bool _bIsReady;

	UPROPERTY(Replicated)
	FString _PlayerName;
};
