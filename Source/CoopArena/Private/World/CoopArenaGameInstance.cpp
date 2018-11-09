// Fill out your copyright notice in the Description page of Project Settings.

#include "World/CoopArenaGameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"


UCoopArenaGameInstance::UCoopArenaGameInstance()
{
	m_MapToHost = "Lobby";
	m_SessionName = "My session";

	m_SessionSettings.bIsLANMatch = true;
	m_SessionSettings.NumPublicConnections = 6;
	m_SessionSettings.bShouldAdvertise = true;
	m_SessionSettings.bIsDedicated = IsDedicatedServerInstance();
	m_SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
	m_SessionSettings.bAllowInvites = true;
	m_SessionSettings.bAllowJoinInProgress = true;
	m_SessionSettings.bAllowJoinViaPresence = true;	

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
	m_SessionInterface->CreateSession(0, m_SessionName, m_SessionSettings);
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

	m_SessionSearch = MakeShareable(new FOnlineSessionSearch());
	m_SessionSearch->bIsLanQuery = true;

	if (IsDedicatedServerInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("We are a dedicated server, hosting the lobby map."));
		Host("Lobby");
	}
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::SearchForGames()
{
	m_bWantsToSearchForGames = true;	

	if (m_SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Start searching for other sessions."));
		m_SessionInterface->FindSessions(0, m_SessionSearch.ToSharedRef());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Session search is not valid. Creating new one. "));
		m_SessionSearch = MakeShareable(new FOnlineSessionSearch());
		m_SessionSearch->bIsLanQuery = true;
	}
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
		GetEngine()->AddOnScreenDebugMessage(0, 5.0f, FColor::Blue, "Found session", true);
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
