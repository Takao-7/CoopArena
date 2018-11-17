// Fill out your copyright notice in the Description page of Project Settings.

#include "World/CoopArenaGameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSessionSettings.h"

const static FName SESSION_NAME = "CoopArena Session";


UCoopArenaGameInstance::UCoopArenaGameInstance()
{
	m_MapToHost = "Lobby";
	m_SessionName = SESSION_NAME;

	m_bWantsToSearchForGames = false;
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::Host(FString Map)
{
	m_MapToHost = Map;

	auto existingSession = m_SessionInterface->GetNamedSession(m_SessionName);
	existingSession ? m_SessionInterface->DestroySession(m_SessionName) : CreateSession();	
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::CreateSession()
{
	FOnlineSessionSettings sessionSettings;
	sessionSettings.bIsLANMatch = true;
	sessionSettings.NumPublicConnections = 6;
	sessionSettings.bShouldAdvertise = true;
	//sessionSettings.Set(FSessionSettingKeys::SessionName, m_SessionName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	if (m_SessionInterface->GetNamedSession(m_SessionName))
	{
		m_SessionInterface->DestroySession(m_SessionName);
	}

	m_SessionInterface->CreateSession(0, m_SessionName, sessionSettings);
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::Join(const FString& IpAdress)
{
	const FString adressToTravel = (IpAdress == "" || IpAdress.IsEmpty()) ? "127.0.0.1" : IpAdress;
	GetFirstLocalPlayerController()->ClientTravel(adressToTravel, ETravelType::TRAVEL_Absolute);
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::Init()
{
	auto onlineSubsystem = IOnlineSubsystem::Get();
	ensureMsgf(onlineSubsystem, TEXT("No subsystem found!"));

	m_SessionInterface = onlineSubsystem->GetSessionInterface();
	ensureMsgf(m_SessionInterface.IsValid(), TEXT("No session interface."));
	m_SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UCoopArenaGameInstance::OnCreateSessionComplete);
	m_SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UCoopArenaGameInstance::OnDestroySessionComplete);
	m_SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UCoopArenaGameInstance::OnFindSessionComplete);
	m_SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UCoopArenaGameInstance::OnJoinSessionComplete);

	m_SessionSearch = MakeShareable(new FOnlineSessionSearch());
	m_SessionSearch->bIsLanQuery = true;
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::SearchForGames()
{
	m_bWantsToSearchForGames = true;	

	if (m_SessionSearch.IsValid() == false)
	{
		UE_LOG(LogTemp, Error, TEXT("Session search is not valid. Creating new one. "));
		m_SessionSearch = MakeShareable(new FOnlineSessionSearch());
		m_SessionSearch->bIsLanQuery = true;
	}

	UE_LOG(LogTemp, Warning, TEXT("Start searching for other sessions."));
	m_SessionInterface->FindSessions(0, m_SessionSearch.ToSharedRef());
}

void UCoopArenaGameInstance::StopSearchingForGames()
{
	m_bWantsToSearchForGames = false;
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::OnCreateSessionComplete(FName SessionName, bool bSuccess)
{
	if (bSuccess)
	{
		const FString mapToLoad = m_MapToHost.IsEmpty() ? "Lobby" : m_MapToHost;
		UE_LOG(LogTemp, Warning, TEXT("Session successfully created. Loading level: %s"), *mapToLoad);
		UGameplayStatics::OpenLevel(GetWorld(), *("/Game/Maps/" + mapToLoad), true, "listen");
	}
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
	if (bSuccess && m_SessionSearch->SearchResults.Num() > 0)
	{
		GetEngine()->AddOnScreenDebugMessage(0, 5.0f, FColor::Green, "Found session", true);
		for (const FOnlineSessionSearchResult& result : m_SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found session id: %s"), *result.GetSessionIdStr());
		}
	}
	else
	{
		GetEngine()->AddOnScreenDebugMessage(0, 5.0f, FColor::Red, "No sessions found", true);
	}

	if (m_bWantsToSearchForGames)
	{
		SearchForGames();
	}
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	FString adress;
	const bool bFoundString = m_SessionInterface->GetResolvedConnectString(SessionName, adress);
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
	m_SessionInterface->JoinSession(0, m_SessionName, m_SessionSearch->SearchResults[SearchResultIndex]);
}
