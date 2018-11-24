// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSessionInterface.h"
#include "CoopArenaGameInstance.generated.h"


USTRUCT(BlueprintType)
struct FSessionData
{
	GENERATED_BODY()

public:
	FSessionData() {};
	FSessionData(FString MatchName, FString PlayerName, FString SessionId, int32 Ping, int32 MaxPlayers, int32 ConnectedPlayer)
	{
		this->MatchName = MatchName;
		this->PlayerName = PlayerName;
		this->SessionId = SessionId;
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
	FString SessionId;

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

	/* Create a new session on the current map. Will destroy the current session and create a new one if it exists. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Game Mode")
	void CreateSession(FString MatchName = "My Match");

	/**
	 * Joins the game on a IP-Address or travel to a map.
	 * @param Address	Can be an IP-Address or a map name.
	 * If it's an IP-Address, then we will try to join the game on this IP-address.
	 * If it's a map name, then we will (client) travel to that map.
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Game Mode")
	void Join(const FString& Address);

	/** virtual function to allow custom GameInstances an opportunity to set up what it needs */
	virtual void Init() override;

	/* Starts searching for LAN games */
	UFUNCTION(Exec, BlueprintCallable, Category = "Game Mode")
	void SearchForGames();

	/* Stops searching for games */
	UFUNCTION(Exec, BlueprintCallable, Category = "Game Mode")
	void StopSearchForGames();

	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void StartMatch(FName MapName = "Level4");

	UPROPERTY(BlueprintAssignable, Category = "Game Mode")
	FOnSessionFound_Event SessionFound_Event;

	int32 GetNumberOfConnectedPlayers() const;

private:
	IOnlineSessionPtr _SessionInterface;
	FString _PlayerName;

	TSharedPtr<class FOnlineSessionSearch> _SessionSearch;
	bool _bWantsToSearchForGames;

	void OnCreateSessionComplete(FName SessionName, bool bSuccess);
	void OnDestroySessionComplete(FName SessionName, bool bSuccess);
	void OnFindSessionComplete(bool bSuccess);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void JoinServer(int32 SearchResultIndex);
};
