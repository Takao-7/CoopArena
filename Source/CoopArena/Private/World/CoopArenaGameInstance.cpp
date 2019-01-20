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
#include "MoviePlayer.h"
#include "WidgetLayoutLibrary.h"
#include "UserWidget.h"
#include "TimerManager.h"
#include "MyGameState.h"


#define SETTING_PlayerName FName("PlayerName")

const FString ARENA_MAP = TEXT("Arena");
const FString MAP_FOLDER = TEXT("/Game/Maps/Menu/");
const FString LOBBY_MAP = TEXT("LobbyMenu");
const FString TRANSITION_MAP = TEXT("Transition");


/////////////////////////////////////////////////////
UCoopArenaGameInstance::UCoopArenaGameInstance()
{
	_bWantsToSearchForGames = false;
	_bWantsToCreateNewSession = false;
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::Init()
{
	SetOnlineMode(EOnlineMode::Offline);

	auto onlineSubsystem = IOnlineSubsystem::Get();
	ensureMsgf(onlineSubsystem, TEXT("No subsystem found!"));

	_SessionInterface = onlineSubsystem->GetSessionInterface();
	ensureMsgf(_SessionInterface.IsValid(), TEXT("No session interface."));

	_SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UCoopArenaGameInstance::OnCreateSessionComplete);
	_SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UCoopArenaGameInstance::OnDestroySessionComplete);
	_SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UCoopArenaGameInstance::OnFindSessionComplete);
	_SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UCoopArenaGameInstance::OnJoinSessionComplete);

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UCoopArenaGameInstance::HandleOnPreLoadMap);
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::Host(FString MapName /*= ""*/, FString PlayerName /*= TEXT("No Name")*/)
{
	SetPlayerName(PlayerName);
	SetOnlineMode(EOnlineMode::LAN);

	const FString map = MapName.IsEmpty() ? GetWorld()->GetMapName() : MapName;
	const FString options = FString(TEXT("?listen"));
	const FString url = MAP_FOLDER + map + options;
	GetWorld()->ServerTravel(url);
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::CreateSession(FString PlayerName /*= ""*/)
{
	SetPlayerName(PlayerName);
	SetOnlineMode(EOnlineMode::LAN);

	if (_SessionInterface->GetNamedSession(NAME_GameSession))
	{
		_bWantsToCreateNewSession = true;
		_SessionInterface->DestroySession(NAME_GameSession);
	}

	FOnlineSessionSettings sessionSettings;
	sessionSettings.bIsLANMatch = true;
	sessionSettings.bUsesPresence = true;
	sessionSettings.NumPublicConnections = 6;
	sessionSettings.NumPrivateConnections = 0;
	sessionSettings.bAllowInvites = true;
	sessionSettings.bAllowJoinInProgress = true;
	sessionSettings.bShouldAdvertise = true;
	sessionSettings.bAllowJoinViaPresence = true;
	sessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
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

	GetWorld()->GetAuthGameMode()->bUseSeamlessTravel = false;
	UGameplayStatics::OpenLevel(GetWorld(), "/Game/Maps/Menu/LobbyMenu", true, "listen");
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::DestroySession()
{
	if (_SessionInterface->GetNamedSession(NAME_GameSession))
	{
		if (_SessionInterface->GetNamedSession(NAME_GameSession)->bHosting)
		{
			AMyGameState* gameState = GetWorld()->GetGameState<AMyGameState>();
			if (gameState)
			{
				gameState->EndMatch();
			}
		}

		_bWantsToCreateNewSession = false;
		_SessionInterface->DestroySession(NAME_GameSession);
	}
}

void UCoopArenaGameInstance::OnDestroySessionComplete(FName SessionName, bool bSuccess)
{
	if (_bWantsToCreateNewSession)
	{
		_bWantsToCreateNewSession = false;
		CreateSession();
	}
	else
	{
		UGameplayStatics::OpenLevel(GetWorld(), "/Game/Maps/Menu/MainMenu", true);
	}
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::SetPlayerName(FString PlayerName)
{
	if (PlayerName.IsEmpty())
	{
		return;
	}

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
		_SessionSearch->bIsLanQuery = true;
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
			FString playerName = "No player name";
			result.Session.SessionSettings.Get(SETTING_PlayerName, playerName);
			const int32 Ping = result.PingInMs;
			const int32 MaxPlayers = result.Session.SessionSettings.NumPublicConnections;
			const int32 ConnectedPlayer = MaxPlayers - result.Session.NumOpenPublicConnections;
			searchResult.Add(FSessionData(playerName, Ping, MaxPlayers, ConnectedPlayer));
		}
		OnSessionFound.Broadcast(searchResult);
		GetEngine()->AddOnScreenDebugMessage(INDEX_NONE, 2.0f, FColor::Green, TEXT("Found games!"));
	}
	else
	{
		if (_SessionSearch->SearchResults.Num() == 0)
		{
			GetEngine()->AddOnScreenDebugMessage(INDEX_NONE, 2.0f, FColor::Red, TEXT("No games found!"));
		}

		if (!bSuccess)
		{
			GetEngine()->AddOnScreenDebugMessage(INDEX_NONE, 2.0f, FColor::Red, TEXT("Searching not successfull!"));
		}
	}

	if (_bWantsToSearchForGames)
	{
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
	ShowLoadingScreen();

	GetWorld()->GetAuthGameMode()->bUseSeamlessTravel = true;
	_SessionInterface->StartSession(NAME_GameSession);

	const FString url = TEXT("/Game/Maps/") + MapName;
	GetWorld()->ServerTravel(url);
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::ShowLoadingScreen()
{
	AMyGameState* gameState = GetWorld()->GetGameState<AMyGameState>();
	ensure(gameState);
	gameState->ShowLoadingScreen_Multicast();
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
void UCoopArenaGameInstance::HandleOnPreLoadMap(const FString& MapName)
{
	if (!IsRunningDedicatedServer())
	{
		UWidgetLayoutLibrary::RemoveAllWidgets(GetPrimaryPlayerController());

		FLoadingScreenAttributes LoadingScreen;
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
		LoadingScreen.WidgetLoadingScreen = FLoadingScreenAttributes::NewTestLoadingScreenWidget();

		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);		
	}
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::RestartLevel(const FString& MapName)
{
	UWorld* world = GetWorld();
	AGameModeBase* gameMode = world->GetAuthGameMode();

	ShowLoadingScreen();

	world->GetTimerManager().ClearAllTimersForObject(gameMode);

	gameMode->bUseSeamlessTravel = true;
	const FString url = TEXT("/Game/Maps/") + MapName;
	world->ServerTravel(url);
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::LeaveMatch()
{
	AGameModeBase* gameMode = GetWorld()->GetAuthGameMode();
	if (gameMode)	// We are the server. Destroy the session so that every client can travel to the main menu.
	{		
		gameMode->EndPlay(EEndPlayReason::Quit);
		DestroySession();		
	}
	else
	{
		DestroySession();
		//UGameplayStatics::OpenLevel(GetWorld(), "/Game/Maps/Menu/MainMenu", true);
	}
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::SetOnlineMode(EOnlineMode OnlineMode)
{
	_OnlineMode = OnlineMode;
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

void UCoopArenaGameInstance::Join(const FString& Address, FString PlayerName /*= ""*/)
{
	const bool bValidAdress = !Address.IsEmpty();
	StopSearchForGames();

	SetOnlineMode(bValidAdress ? EOnlineMode::LAN : EOnlineMode::Offline);
	SetPlayerName(PlayerName);

	const FString addressToTravel = bValidAdress ? Address : "127.0.0.1";
	FString text = "Joining: " + addressToTravel;
	GetEngine()->AddOnScreenDebugMessage(INDEX_NONE, 2.0f, FColor::Green, text);
	GetFirstLocalPlayerController()->ClientTravel(addressToTravel, ETravelType::TRAVEL_Absolute);
}