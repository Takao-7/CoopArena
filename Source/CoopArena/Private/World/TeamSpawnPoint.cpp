// Fill out your copyright notice in the Description page of Project Settings.

#include "TeamSpawnPoint.h"
#include "Components/ChildActorComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/World.h"


ATeamSpawnPoint::ATeamSpawnPoint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	m_NumSpawnPoints = 6;


}
