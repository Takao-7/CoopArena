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
	FSessionData(FName MatchName, FString PlayerName, FString SessionId, int32 Ping, int32 MaxPlayers, int32 ConnectedPlayer)
	{
		this->matchName = MatchName;
		this->playerName = PlayerName;
		this->sessionId = SessionId;
		this->ping = Ping;
		this->maxPlayers = MaxPlayers;
		this->connectedPlayers = ConnectedPlayer;
	};

	UPROPERTY(BlueprintReadWrite)
	FName matchName;

	UPROPERTY(BlueprintReadWrite)
	FString playerName;

	UPROPERTY(BlueprintReadWrite)
	FString sessionId;

	UPROPERTY(BlueprintReadWrite)
	int32 ping;

	UPROPERTY(BlueprintReadWrite)
	int32 maxPlayers;

	UPROPERTY(BlueprintReadWrite)
	int32 connectedPlayers;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionFound_Event, const TArray<FSessionData>&, FoundSessions);


UCLASS()
class COOPARENA_API UCoopArenaGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UCoopArenaGameInstance();

	/* Hosts a server at the given map or, when no map given, at the 'Lobby' map. Creates also a new sesssion. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Game Mode")
	void Host(FString Map);

	/* Create a new session on the current map. Will destroy the current session if it exists. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Game Mode")
	void CreateSession(FString MatchName = "My Match");

	/* Loads the given map or joins the given IP-Address. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Game Mode")
	void Join(const FString& IpAdress);

	/** virtual function to allow custom GameInstances an opportunity to set up what it needs */
	virtual void Init() override;

	/* Starts searching for LAN games */
	UFUNCTION(Exec, BlueprintCallable, Category = "Game Mode")
	void SearchForGames();

	/* Stops searching for games */
	UFUNCTION(Exec, BlueprintCallable, Category = "Game Mode")
	void StopSearchingForGames();

	UPROPERTY(BlueprintAssignable, Category = "Game Mode")
	FOnSessionFound_Event OnSessionFound;

private:
	IOnlineSessionPtr m_SessionInterface;
	FString m_MapToHost;
	FString m_PlayerName;

	TSharedPtr<class FOnlineSessionSearch> m_SessionSearch;
	bool m_bWantsToSearchForGames;

	void OnCreateSessionComplete(FName SessionName, bool bSuccess);
	void OnDestroySessionComplete(FName SessionName, bool bSuccess);
	void OnFindSessionComplete(bool bSuccess);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void JoinServer(int32 SearchResultIndex);
};
