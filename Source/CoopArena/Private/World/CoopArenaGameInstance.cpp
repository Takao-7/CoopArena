// Fill out your copyright notice in the Description page of Project Settings.

#include "World/CoopArenaGameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "CoopArenaGameMode.h"
#include "UnrealNames.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "MyPlayerState.h"


#define SETTING_ServerName FName("MatchName")
#define SETTING_PlayerName FName("PlayerName")
const FString ARENA_MAP = TEXT("Level6");


UCoopArenaGameInstance::UCoopArenaGameInstance()
{
	_bWantsToSearchForGames = false;
	_bWantsToCreateNewSession = false;
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
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::Host(FString ServerName /*= TEXT("No-Name Server")*/, FString PlayerName /*= TEXT("Nobody")*/)
{
	_ServerName = ServerName;
	SetPlayerName(PlayerName);

	if (_SessionInterface->GetNamedSession(NAME_GameSession))
	{
		_bWantsToCreateNewSession = true;
		_SessionInterface->DestroySession(NAME_GameSession);
	}
	else
	{
		CreateSession();
	}
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::CreateSession()
{
	FOnlineSessionSettings sessionSettings;
	sessionSettings.bIsLANMatch = true;
	sessionSettings.NumPublicConnections = 6;
	sessionSettings.bShouldAdvertise = true;
	sessionSettings.bUsesPresence = true;
	sessionSettings.Set(SETTING_ServerName, _ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	sessionSettings.Set(SETTING_PlayerName, _PlayerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	_SessionInterface->CreateSession(0, NAME_GameSession, sessionSettings);
}

void UCoopArenaGameInstance::OnCreateSessionComplete(FName SessionName, bool bSuccess)
{
	if (!bSuccess)
	{
		GetEngine()->AddOnScreenDebugMessage(INDEX_NONE, 2.0f, FColor::Red, TEXT("Session not created!"));
		return;
	}

	const FString mapFolder = FString(TEXT("/Game/Maps/Menu/"));
	const FString lobbyMap = FString(TEXT("LobbyMenu"));
	const FString options = FString(TEXT("?listen"));
	const FString url = mapFolder + lobbyMap + options;
	GetWorld()->ServerTravel(url);
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::DestroySession()
{
	if (_SessionInterface->GetNamedSession(NAME_GameSession))
	{
		_bWantsToCreateNewSession = false;
		_SessionInterface->DestroySession(NAME_GameSession);
	}
}

void UCoopArenaGameInstance::OnDestroySessionComplete(FName SessionName, bool bSuccess)
{
	if (bSuccess && _bWantsToCreateNewSession)
	{
		_bWantsToCreateNewSession = false;
		CreateSession();
	}
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::SetPlayerName(FString PlayerName)
{
	_PlayerName = PlayerName;
	APlayerState* playerState = GetPrimaryPlayerController()->PlayerState;
	if(playerState)
	{
		playerState->SetPlayerName(PlayerName);
	}
	else
	{
		GetEngine()->AddOnScreenDebugMessage(INDEX_NONE, 2.0f, FColor::Red, TEXT("No player state, when trying to set MyName!"));
	}
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::SearchForGames()
{
	_bWantsToSearchForGames = true;
	if (_SessionSearch.IsValid() == false)
	{
		_SessionSearch = MakeShareable(new FOnlineSessionSearch());
		_SessionSearch->MaxSearchResults = 100;
		_SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	}
	_SessionInterface->FindSessions(0, _SessionSearch.ToSharedRef());
	GetEngine()->AddOnScreenDebugMessage(INDEX_NONE, 2.0f, FColor::Green, TEXT("Start searching for games"));
}

void UCoopArenaGameInstance::OnFindSessionComplete(bool bSuccess)
{
	if (bSuccess && _SessionSearch->SearchResults.Num() > 0 && _bWantsToSearchForGames)
	{
		TArray<FSessionData> searchResult;
		for (const FOnlineSessionSearchResult& result : _SessionSearch->SearchResults)
		{
			FString matchName = "No name";
			result.Session.SessionSettings.Get(SETTING_ServerName, matchName);
			FString playerName = "No player name";
			result.Session.SessionSettings.Get(SETTING_PlayerName, playerName);
			const int32 Ping = result.PingInMs;
			const int32 MaxPlayers = result.Session.SessionSettings.NumPublicConnections;
			const int32 ConnectedPlayer = MaxPlayers - result.Session.NumOpenPublicConnections;
			searchResult.Add(FSessionData(matchName, playerName, Ping, MaxPlayers, ConnectedPlayer));
		}
		OnSessionFound.Broadcast(searchResult);
		GetEngine()->AddOnScreenDebugMessage(INDEX_NONE, 2.0f, FColor::Green, TEXT("Found games!"));
		
		SearchForGames();
	}
}

void UCoopArenaGameInstance::StopSearchForGames()
{
	_bWantsToSearchForGames = false;
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::StartMatch(FString MapName /*= ARENA_MAP*/)
{
	_SessionInterface->StartSession(NAME_GameSession);
	
	const FString url = TEXT("/Game/Maps/") + MapName;
	GetWorld()->ServerTravel(url);
}

/////////////////////////////////////////////////////
int32 UCoopArenaGameInstance::GetNumberOfConnectedPlayers() const
{
	FNamedOnlineSession* session = _SessionInterface->GetNamedSession(NAME_GameSession);
	if (session == nullptr)
	{
		return -1;
	}
	const int32 maxPlayers = session->SessionSettings.NumPublicConnections;
	return maxPlayers - session->NumOpenPublicConnections;
}

/////////////////////////////////////////////////////
FString UCoopArenaGameInstance::GetSavedPlayerName() const
{
	return _PlayerName;
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::EndMatch()
{
	DestroySession();
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::JoinServer(int32 SearchResultIndex, FString PlayerName /*= TEXT("Nobody")*/)
{
	ensureMsgf(_SessionSearch->SearchResults.IsValidIndex(SearchResultIndex), TEXT("The given index is not valid."));
	StopSearchForGames();
	SetPlayerName(PlayerName);
	_SessionInterface->JoinSession(0, NAME_GameSession, _SessionSearch->SearchResults[SearchResultIndex]);
}

void UCoopArenaGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	FString address;
	const bool bFoundString = _SessionInterface->GetResolvedConnectString(SessionName, address);
	if (!bFoundString)
	{
		UE_LOG(LogTemp, Error, TEXT("No resolved connect string found."));
		return;
	}
	Join(address);
}

void UCoopArenaGameInstance::Join(const FString& Address)
{
	const FString addressToTravel = Address.IsEmpty() ? "127.0.0.1" : Address;
	FString text = "Joining: " + addressToTravel;
	GetEngine()->AddOnScreenDebugMessage(INDEX_NONE, 2.0f, FColor::Green, text);
	GetFirstLocalPlayerController()->ClientTravel(addressToTravel, ETravelType::TRAVEL_Absolute);
}