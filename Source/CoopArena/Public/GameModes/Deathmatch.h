// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoopArenaGameMode.h"
#include "Deathmatch.generated.h"


UCLASS()
class COOPARENA_API ADeathmatch : public ACoopArenaGameMode
{
	GENERATED_BODY()

private:
	int32 m_CurrentTeamIndex;

public:
	ADeathmatch();
	virtual void RegisterPlayer(APlayerController* Controller) override;	

	void SetTeamName(AController* Controller);

	virtual void RegisterBot(AController* Controller) override;
};
