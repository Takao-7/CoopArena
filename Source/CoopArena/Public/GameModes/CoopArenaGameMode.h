// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CoopArenaGameMode.generated.h"


class ASpawnPoint;
class APlayerController;
class AController;


UCLASS()
class COOPARENA_API ACoopArenaGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	
protected:
	UPROPERTY(BlueprintReadWrite, Category = "CoopArena game mode")
	TArray<ASpawnPoint*> SpawnPoints;

	UPROPERTY(BlueprintReadWrite, Category = "CoopArena game mode")
	TArray<APlayerController*> Players;

	UPROPERTY(BlueprintReadWrite, Category = "CoopArena game mode")
	TArray<AController*> Bots;

	UFUNCTION(BlueprintCallable, Category = "CoopArena game mode")
	void FindSpawnPoints();

	UPROPERTY(EditDefaultsOnly, Category = "CoopArena game mode", meta = (DisplayName = "Default player team"))
	FName m_DefaultPlayerTeam;

	UPROPERTY(EditDefaultsOnly, Category = "CoopArena game mode", meta = (DisplayName = "Default bot team"))
	FName m_DefaultBotTeam;

public:
	ACoopArenaGameMode();

	/**
	* Checks if the given Controller has any tag that contains 'Team'.
	* @return The first tag containing 'Team' or an empty FName if no tag was found.
	*/
	FString CheckForTeamTag(const AController& Controller) const;

	UFUNCTION(BlueprintCallable, Category = "CoopArena game mode")
	virtual void RegisterPlayer(APlayerController* Controller);

	UFUNCTION(BlueprintCallable, Category = "CoopArena game mode")
	virtual void RegisterBot(AController* Controller);

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	/** Called after a successful login.  This is the first place it is safe to call replicated functions on the PlayerController. */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
};