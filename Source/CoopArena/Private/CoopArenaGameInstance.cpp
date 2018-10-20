// Fill out your copyright notice in the Description page of Project Settings.

#include "CoopArenaGameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"


/////////////////////////////////////////////////////
void UCoopArenaGameInstance::Host(const FString& Map)
{
	const FString mapToLoad = Map.IsEmpty() ? GetWorld()->GetMapName() : Map;
	UGameplayStatics::OpenLevel(GetWorld(), *("/Game/Maps/" + mapToLoad), true, "listen");
}

void UCoopArenaGameInstance::HostLobby()
{
	UGameplayStatics::OpenLevel(GetWorld(), "/Game/Maps/Lobby", true, "listen");
}

/////////////////////////////////////////////////////
void UCoopArenaGameInstance::Join(const FString& IpAdress)
{
	const FString adressToTravel = (IpAdress == "" || IpAdress.IsEmpty()) ? "127.0.0.1" : IpAdress;
	UGameplayStatics::OpenLevel(GetWorld(), *adressToTravel, true);
}
