// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "CoopArenaGameInstance.generated.h"


USTRUCT(BlueprintType)
struct FSessionData
{
	GENERATED_BODY()

public:
	FSessionData() {};
	FSessionData(FString MatchName, FString PlayerName, int32 Ping, int32 MaxPlayers, int32 ConnectedPlayer)
	{
		this->MatchName = MatchName;
		this->PlayerName = PlayerName;
		this->Ping = Ping;
		this->MaxPlayers = MaxPlayers;
		this->ConnectedPlayers = ConnectedPlayer;
	};

	/* The match name that this session's creator has chosen. This is NOT the session name. */
	UPROPERTY(BlueprintReadWrite)
	FString MatchName;

	/* The in game name of this session's creator. */
	UPROPERTY(BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(BlueprintReadWrite)
	int32 Ping;

	/* The maximum number of players that can connect to this session. */
	UPROPERTY(BlueprintReadWrite)
	int32 MaxPlayers;

	/* Number of players that are currently connected to this session. */
	UPROPERTY(BlueprintReadWrite)
	int32 ConnectedPlayers;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionFound_Event, const TArray<FSessionData>&, FoundSessions);


UCLASS()
class COOPARENA_API UCoopArenaGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UCoopArenaGameInstance();

	/* Create a new session. Will destroy the current session and create a new one if it exists. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Game instance")
	void CreateSession();

	/* Destroy the current session. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Game instance")
	void DestroySession();

	UFUNCTION(Exec, BlueprintCallable, Category = "Game instance")
	void Host(FString ServerName = TEXT("No-Name Server"), FString PlayerName = TEXT("Nobody"));

	/**
	 * Joins the game on a IP-Address or travel to a map.
	 * @param Address	Can be an IP-Address or a map name.
	 * If it's an IP-Address, then we will try to join the game on this IP-address.
	 * If it's a map name, then we will (client) travel to that map.
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Game instance")
	void Join(const FString& Address);

	/** virtual function to allow custom GameInstances an opportunity to set up what it needs */
	virtual void Init() override;

	/* Starts searching for LAN games */
	UFUNCTION(Exec, BlueprintCallable, Category = "Game instance")
	void SearchForGames();

	/* Stops searching for games */
	UFUNCTION(Exec, BlueprintCallable, Category = "Game instance")
	void StopSearchForGames();

	UFUNCTION(BlueprintCallable, Category = "Game instance")
	void StartMatch(FString MapName = "Level4");

	/* This event will be called each time OnFindSessionComplete delegate is fired AND we actually found sessions. */
	UPROPERTY(BlueprintAssignable, Category = "Game instance")
	FOnSessionFound_Event OnSessionFound;

	/**
	 * Returns the number of connected players.
	 * If there is no session, then we return -1.
	 */
	UFUNCTION(BlueprintPure, Category = "Game instance")
	int32 GetNumberOfConnectedPlayers() const;

	/**
	 * Returns the saved player name, which was entered in the host or join game menu and saved here in the game instance.
	 * This is necessary, because whenever we travel non-seamless the PlayerName in PlayerState is reset.
	 */
	UFUNCTION(BlueprintPure, Category = "Game instance")
	FString GetSavedPlayerName() const;

	UFUNCTION(BlueprintCallable, Category = "Game instance")
	void EndMatch();

private:
	IOnlineSessionPtr _SessionInterface;
	TSharedPtr<class FOnlineSessionSearch> _SessionSearch;
	bool _bWantsToSearchForGames;
	bool _bWantsToCreateNewSession;
	FString _PlayerName;
	FString _ServerName;

	void OnCreateSessionComplete(FName SessionName, bool bSuccess);
	void OnDestroySessionComplete(FName SessionName, bool bSuccess);
	void OnFindSessionComplete(bool bSuccess);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION(BlueprintCallable, Category = "Game instance")
	void JoinServer(int32 SearchResultIndex, FString PlayerName = TEXT("Nobody"));

	void SetPlayerName(FString PlayerName);
};
