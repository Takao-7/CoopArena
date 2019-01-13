// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "CoopArenaGameMode.generated.h"


class ASpawnPoint;
class APlayerController;
class AController;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerDeath_Signature, APlayerCharacter*, PlayerThatDied, AController*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBotDeath_Signature, AActor*, BotThatDied, AController*, Killer);


UCLASS()
class COOPARENA_API ACoopArenaGameMode : public AGameMode
{
	GENERATED_BODY()
	
protected:
	/* All spawn points on the map */
	UPROPERTY(BlueprintReadOnly, Category = "CoopArena game mode")
	TArray<ASpawnPoint*> _spawnPoints;

	/* The number of players that are currently alive */
	UPROPERTY(BlueprintReadOnly, Category = "CoopArena game mode")
	int32 _numPlayersAlive;

	/* A list of all player characters on the map (dead an alive) */
	UPROPERTY(BlueprintReadOnly, Category = "CoopArena game mode")
	TArray<APlayerCharacter*> _playerCharacters;

	/* A list of all alive player characters on the map */
	UPROPERTY(BlueprintReadOnly, Category = "CoopArena game mode")
	TArray<APlayerCharacter*> _playerCharactersAlive;

	/* A list of all player controllers on the map */
	UPROPERTY(BlueprintReadOnly, Category = "CoopArena game mode")
	TArray<APlayerController*> _playerControllers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CoopArena game mode")
	FName _defaultPlayerTeam;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CoopArena game mode")
	FName _defaultBotTeam;

	UFUNCTION(BlueprintCallable, Category = "CoopArena game mode")
	void FindSpawnPoints();

public:
	ACoopArenaGameMode();

	UPROPERTY(BlueprintAssignable, Category = "CoopArena game mode")
	FOnPlayerDeath_Signature PlayerDeath_Event;

	UPROPERTY(BlueprintAssignable, Category = "CoopArena game mode")
	FOnBotDeath_Signature BotDeath_Event;

	/**
	* Checks if the given Controller has any tag that contains 'Team'.
	* @return The first tag containing 'Team' or an empty FName if no tag was found.
	*/
	FString CheckForTeamTag(const AController& Controller) const;

	void RegisterPlayerCharacter(APlayerCharacter* PlayerCharacter);
	void UnregisterPlayerCharacter(APlayerCharacter* PlayerCharacter);

	/**
	 * Checks if the given controller or actor is allowed to re-spawn.
	 * At least one of the two parameters must be not null.
	 * @param PlayerController The player controller that wants to re-spawn
	 * @param Actor The actor that wants to re-spawn.
	 * @return If the given controller or actor is allowed to re-spawn.
	 */
	bool CanRespawn(APlayerController* PlayerController, AActor* Actor) const;

	UFUNCTION(BlueprintPure, Category = "CoopArena game mode")
	FName GetPlayerTeamName() const { return _defaultPlayerTeam; };

	UFUNCTION(BlueprintPure, Category = "CoopArena game mode")
	FName GetBotTeamName() const { return _defaultBotTeam; };

	UFUNCTION(Exec)
	void DestroyAllBots();


	/////////////////////////////////////////////////////
				/* Overridden functions */
	/////////////////////////////////////////////////////
public:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/**
	 * Return the specific player start actor that should be used for the next spawn
	 * This will either use a previously saved start actor, or calls ChoosePlayerStart
	 *
	 * @param Player The AController for whom we are choosing a Player Start
	 * @param IncomingName Specifies the tag of a Player Start to use
	 * @returns Actor chosen as player start (usually a PlayerStart)
	 */
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName = TEXT("")) override;

	/** Called after a successful login.  This is the first place it is safe to call replicated functions on the PlayerController. */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;
};