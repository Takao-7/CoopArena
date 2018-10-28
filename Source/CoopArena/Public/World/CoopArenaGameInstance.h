// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "CoopArenaGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class COOPARENA_API UCoopArenaGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UCoopArenaGameInstance();

	/* Hosts a server at the given map or, when no map given, at the 'Lobby' map. */
	UFUNCTION(Exec)
	void Host(const FString& Map);

	void CreateSession();

	/* Loads the given map or joins the given IP-Address. */
	UFUNCTION(Exec)
	void Join(const FString& IpAdress);

	/** virtual function to allow custom GameInstances an opportunity to set up what it needs */
	virtual void Init() override;

private:
	IOnlineSessionPtr m_SessionInterface;
	FString m_MapToHost;
	FName m_SessionName;
	TSharedPtr<class FOnlineSessionSearch> m_SessionSearch;

	void OnCreateSessionComplete(FName SessionName, bool bSuccess);
	void OnDestroySessionComplete(FName SessionName, bool bSuccess);
	void OnFindSessionComplete(bool bSuccess);
};
