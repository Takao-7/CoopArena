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

	void SetTeamName(AController* Controller);
};
