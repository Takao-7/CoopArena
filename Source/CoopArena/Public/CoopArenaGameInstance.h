// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CoopArenaGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class COOPARENA_API UCoopArenaGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	/* Hosts a server at the given map or, when no map given, at the current map. */
	UFUNCTION(Exec)
	void Host(const FString& Map);

	/* Starts a server at the 'Lobby' map. */
	UFUNCTION(Exec)
	void HostLobby();

	/* Loads the given map or joins the given IP-Adress. */
	UFUNCTION(Exec)
	void Join(const FString& IpAdress);
};
