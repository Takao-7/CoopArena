// Fill out your copyright notice in the Description page of Project Settings.

#include "World/CoopArenaGameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"


#define SETTING_MatchName FName(TEXT("MatchName"))


UCoopArenaGameInstance::UCoopArenaGameInstance()
{
	_bWantsToSearchForGames = false;
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::CreateSession(FString MatchName /*= "My Match"*/)
{
	FOnlineSessionSettings sessionSettings;
	sessionSettings.bIsLANMatch = true;
	sessionSettings.NumPublicConnections = 6;
	sessionSettings.bShouldAdvertise = true;
	sessionSettings.Set(SETTING_MatchName, MatchName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	if (_SessionInterface->GetNamedSession(NAME_GameSession))
	{
		_SessionInterface->DestroySession(NAME_GameSession);
	}

	_SessionInterface->CreateSession(0, NAME_GameSession, sessionSettings);
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::Join(const FString& Address)
{
	const FString adressToTravel = (Address == "" || Address.IsEmpty()) ? "127.0.0.1" : Address;
	GetFirstLocalPlayerController()->ClientTravel(adressToTravel, ETravelType::TRAVEL_Absolute);
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::Init()
{
	auto onlineSubsystem = IOnlineSubsystem::Get();
	ensureMsgf(onlineSubsystem, TEXT("No subsystem found!"));

	_SessionInterface = onlineSubsystem->GetSessionInterface();
	ensureMsgf(_SessionInterface.IsValid(), TEXT("No session interface."));
	_SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UCoopArenaGameInstance::OnCreateSessionComplete);
	_SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UCoopArenaGameInstance::OnDestroySessionComplete);
	_SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UCoopArenaGameInstance::OnFindSessionComplete);
	_SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UCoopArenaGameInstance::OnJoinSessionComplete);

	_SessionSearch = MakeShareable(new FOnlineSessionSearch());
	_SessionSearch->bIsLanQuery = true;
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::SearchForGames()
{
	_bWantsToSearchForGames = true;	

	if (_SessionSearch.IsValid() == false)
	{
		UE_LOG(LogTemp, Error, TEXT("Session search is not valid. Creating new one. "));
		_SessionSearch = MakeShareable(new FOnlineSessionSearch());
		_SessionSearch->bIsLanQuery = true;
	}

	UE_LOG(LogTemp, Warning, TEXT("Start searching for other sessions."));
	_SessionInterface->FindSessions(0, _SessionSearch.ToSharedRef());
}

void UCoopArenaGameInstance::StopSearchForGames()
{
	_bWantsToSearchForGames = false;
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::OnCreateSessionComplete(FName SessionName, bool bSuccess)
{
	FNamedOnlineSession* session = _SessionInterface->GetNamedSession(NAME_GameSession);
	session->OwningUserName = _PlayerName;
}

void UCoopArenaGameInstance::OnDestroySessionComplete(FName SessionName, bool bSuccess)
{
	if (bSuccess)
	{
		CreateSession();
	}
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::OnFindSessionComplete(bool bSuccess)
{
	if (bSuccess && _SessionSearch->SearchResults.Num() > 0)
	{
		TArray<FSessionData> searchResult;
		for (const FOnlineSessionSearchResult& result : _SessionSearch->SearchResults)
		{
			FString matchName = "No name";
			result.Session.SessionSettings.Get(SETTING_MatchName, matchName);
			const FString playerName = result.Session.OwningUserName;
			const FString SessionId = result.GetSessionIdStr();
			const int32 Ping = result.PingInMs;
			const int32 MaxPlayers = result.Session.SessionSettings.NumPublicConnections;
			const int32 ConnectedPlayer = MaxPlayers - result.Session.NumOpenPublicConnections;
			searchResult.Add(FSessionData(NAME_GameSession, playerName, SessionId, Ping, MaxPlayers, ConnectedPlayer));
		}
		OnSessionFound.Broadcast(searchResult);
	}

	if (_bWantsToSearchForGames)
	{
		SearchForGames();
	}
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	FString adress;
	const bool bFoundString = _SessionInterface->GetResolvedConnectString(SessionName, adress);
	if (!bFoundString)
	{
		UE_LOG(LogTemp, Error, TEXT("No resolved connect string found."));
		return;
	}

	Join(adress);
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::JoinServer(int32 SearchResultIndex)
{
	ensureMsgf(_SessionSearch->SearchResults.IsValidIndex(SearchResultIndex), TEXT("The given index is not valid."));
	_SessionInterface->JoinSession(0, NAME_GameSession, _SessionSearch->SearchResults[SearchResultIndex]);
}
