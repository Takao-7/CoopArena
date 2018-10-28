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
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::Host(const FString& Map)
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
	m_SessionInterface->CreateSession(0, m_SessionName, sessionSettings);
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::Join(const FString& IpAdress)
{
	const FString adressToTravel = (IpAdress == "" || IpAdress.IsEmpty()) ? "127.0.0.1" : IpAdress;
	UGameplayStatics::OpenLevel(GetWorld(), *adressToTravel, true);
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::Init()
{
	auto onlineSubsystem = IOnlineSubsystem::Get();
	if (onlineSubsystem == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("No subsystem found!"));
		return;
	}

	m_SessionInterface = onlineSubsystem->GetSessionInterface();
	ensureMsgf(m_SessionInterface.IsValid(), TEXT("No session interface."));
	m_SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UCoopArenaGameInstance::OnCreateSessionComplete);
	m_SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UCoopArenaGameInstance::OnDestroySessionComplete);
	m_SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UCoopArenaGameInstance::OnFindSessionComplete);

	m_SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (m_SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Start searching for other sessions."));
		m_SessionSearch->bIsLanQuery = true;
		m_SessionInterface->FindSessions(0, m_SessionSearch.ToSharedRef());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Session search is not valid."));
	}
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::OnCreateSessionComplete(FName SessionName, bool bSuccess)
{
	if (bSuccess)
	{
		const FString mapToLoad = m_MapToHost.IsEmpty() ? "Lobby" : m_MapToHost;
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
	if (bSuccess)
	{
		for (const FOnlineSessionSearchResult& result : m_SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found session id: %s"), *result.GetSessionIdStr());
		}
	}
}
