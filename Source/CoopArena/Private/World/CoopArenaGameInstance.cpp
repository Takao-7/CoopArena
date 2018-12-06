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


#define SETTING_MatchName FName("MatchName")
#define SETTING_PlayerName FName("PlayerName")


UCoopArenaGameInstance::UCoopArenaGameInstance()
{
	_bWantsToSearchForGames = false;
	_bWantsToCreateSession = false;
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::CreateSession(FString MatchName /*= "My Match"*/, FString PlayerName /*= "Player"*/)
{
	SetPlayerName(PlayerName);

	FOnlineSessionSettings sessionSettings;
	sessionSettings.bIsLANMatch = true;
	sessionSettings.NumPublicConnections = 6;
	sessionSettings.bShouldAdvertise = true;
	sessionSettings.bUsesPresence = true;
	sessionSettings.Set(SETTING_MatchName, MatchName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	sessionSettings.Set(SETTING_PlayerName, PlayerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	if (_SessionInterface->GetNamedSession(NAME_GameSession))
	{
		_SessionInterface->DestroySession(NAME_GameSession);
	}

	_SessionInterface->CreateSession(*GetLocalPlayerByIndex(0)->GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession, sessionSettings);
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::SetPlayerName(FString PlayerName /*= ""*/)
{
	const bool bEmptyString = PlayerName.IsEmpty();
	APlayerState* playerState = GetPrimaryPlayerController()->PlayerState;
	playerState->SetPlayerName(bEmptyString ? _PlayerName : PlayerName);
	if (!bEmptyString)
	{
		_PlayerName = PlayerName;
	}
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
void UCoopArenaGameInstance::StartMatch(FString MapName /*= "Level4"*/)
{
	_SessionInterface->StartSession(NAME_GameSession);
	GetWorld()->ServerTravel(MapName);
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
IOnlineSessionPtr UCoopArenaGameInstance::GetSessionInterface() const
{
	return _SessionInterface;
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::OnCreateSessionComplete(FName SessionName, bool bSuccess)
{
	if (bSuccess)
	{
		GetEngine()->AddOnScreenDebugMessage(INDEX_NONE, 2.0f, FColor::Green, TEXT("Session created without errors!"));
	}
	else
	{
		GetEngine()->AddOnScreenDebugMessage(INDEX_NONE, 2.0f, FColor::Red, TEXT("Session created with errors!"));
	}
	
	GetWorld()->ServerTravel(TEXT("Game/Maps/LobbyMenu?listen?bIsLanMatch=1"));
	//UGameplayStatics::OpenLevel(GetWorld(), "LobbyMenu", false, "listen");
}

void UCoopArenaGameInstance::OnDestroySessionComplete(FName SessionName, bool bSuccess)
{
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
			FString playerName = "No player name";
			result.Session.SessionSettings.Get(SETTING_MatchName, matchName);
			result.Session.SessionSettings.Get(SETTING_PlayerName, playerName);

			const int32 Ping = result.PingInMs;
			const int32 MaxPlayers = result.Session.SessionSettings.NumPublicConnections;
			const int32 ConnectedPlayer = MaxPlayers - result.Session.NumOpenPublicConnections;
			searchResult.Add(FSessionData(matchName, playerName, Ping, MaxPlayers, ConnectedPlayer));
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
	FString address;
	const bool bFoundString = _SessionInterface->GetResolvedConnectString(SessionName, address);
	if (!bFoundString)
	{
		UE_LOG(LogTemp, Error, TEXT("No resolved connect string found."));
		return;
	}
	Join(address);
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::Join(const FString& Address)
{
	const FString addressToTravel = Address.IsEmpty() ? "127.0.0.1" : Address;
	FString text = "Joining: " + addressToTravel;
	GetEngine()->AddOnScreenDebugMessage(INDEX_NONE, 2.0f, FColor::Green, text);
	GetFirstLocalPlayerController()->ClientTravel(addressToTravel, ETravelType::TRAVEL_Absolute);
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::JoinServer(int32 SearchResultIndex)
{
	ensureMsgf(_SessionSearch->SearchResults.IsValidIndex(SearchResultIndex), TEXT("The given index is not valid."));
	StopSearchForGames();
	_SessionInterface->JoinSession(*GetLocalPlayerByIndex(0)->GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession, _SessionSearch->SearchResults[SearchResultIndex]);
}
