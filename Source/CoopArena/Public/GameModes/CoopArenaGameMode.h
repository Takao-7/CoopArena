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
	TArray<ASpawnPoint*> m_SpawnPoints;

	UFUNCTION(BlueprintCallable, Category = "CoopArena game mode")
	void FindSpawnPoints();

	UPROPERTY(EditDefaultsOnly, Category = "CoopArena game mode", meta = (DisplayName = "Default player team"))
	FName m_DefaultPlayerTeam;

	UPROPERTY(EditDefaultsOnly, Category = "CoopArena game mode", meta = (DisplayName = "Default bot team"))
	FName m_DefaultBotTeam;

	uint32 m_NumPlayersAlive;

	TArray<APlayerCharacter*> m_PlayerCharacters;

	TArray<APlayerController*> m_PlayerControllers;

public:
	ACoopArenaGameMode();

	UPROPERTY(BlueprintAssignable, Category = "CoopArena game mode")
	FOnPlayerDeath_Signature OnPlayerDeath_Event;

	UPROPERTY(BlueprintAssignable, Category = "CoopArena game mode")
	FOnBotDeath_Signature OnBotDeath_Event;

	/**
	* Checks if the given Controller has any tag that contains 'Team'.
	* @return The first tag containing 'Team' or an empty FName if no tag was found.
	*/
	FString CheckForTeamTag(const AController& Controller) const;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	void RegisterPlayerCharacter(APlayerCharacter* PlayerCharacter);

	void UnregisterPlayerCharacter(APlayerCharacter* PlayerCharacter);

	/**
	 * Return the specific player start actor that should be used for the next spawn
	 * This will either use a previously saved startactor, or calls ChoosePlayerStart
	 *
	 * @param Player The AController for whom we are choosing a Player Start
	 * @param IncomingName Specifies the tag of a Player Start to use
	 * @returns Actor chosen as player start (usually a PlayerStart)
	 */
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName = TEXT("")) override;

	/** Called after a successful login.  This is the first place it is safe to call replicated functions on the PlayerController. */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	bool CanRespawn(APlayerController* PlayerController);
};