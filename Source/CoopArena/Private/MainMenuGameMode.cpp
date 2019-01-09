// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenuGameMode.h"
#include "HUD/MainMenuHUD.h"


AMainMenuGameMode::AMainMenuGameMode()
{
	HUDClass = AMainMenuHUD::StaticClass();
	bUseSeamlessTravel = false;
}
