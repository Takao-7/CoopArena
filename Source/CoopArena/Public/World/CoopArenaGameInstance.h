// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSessionInterface.h"
#include "CoopArenaGameInstance.generated.h"


//USTRUCT(BlueprintType)
//struct FSessionSettingKeys
//{
//	GENERATED_BODY()
//
//public:
//	static const FName SessionName;
//};


UCLASS()
class COOPARENA_API UCoopArenaGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UCoopArenaGameInstance();

	/* Hosts a server at the given map or, when no map given, at the 'Lobby' map. Creates also a new sesssion. */
	UFUNCTION(Exec)
	void Host(FString Map);

	/* Create a new session on the current map. Will destroy the current session if it exists. */
	UFUNCTION(Exec)
	void CreateSession();

	/* Loads the given map or joins the given IP-Address. */
	UFUNCTION(Exec)
	void Join(const FString& IpAdress);

	/** virtual function to allow custom GameInstances an opportunity to set up what it needs */
	virtual void Init() override;

	/* Starts searching for LAN games */
	UFUNCTION(Exec)
	void SearchForGames();

	/* Stops searching for games */
	UFUNCTION(Exec)
	void StopSearchingForGames();

private:
	IOnlineSessionPtr m_SessionInterface;
	FString m_MapToHost;
	FName m_SessionName;
	TSharedPtr<class FOnlineSessionSearch> m_SessionSearch;
	bool m_bWantsToSearchForGames;

	void OnCreateSessionComplete(FName SessionName, bool bSuccess);
	void OnDestroySessionComplete(FName SessionName, bool bSuccess);
	void OnFindSessionComplete(bool bSuccess);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	void JoinServer(int32 SearchResultIndex);
};
