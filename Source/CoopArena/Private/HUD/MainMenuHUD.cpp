// Fill out your copyright notice in the Description page of Project Settings.

#include "HUD/MainMenuHUD.h"
#include "Blueprint/UserWidget.h"
#include "WidgetBlueprintLibrary.h"
#include "WidgetLayoutLibrary.h"


void AMainMenuHUD::DisplayLoadingScreen(bool bShowLoadingScreen)
{
	if (_LoadingScreenClass)
	{
		if (bShowLoadingScreen)
		{
			UWidgetLayoutLibrary::RemoveAllWidgets(PlayerOwner);
			_LoadingScreen = CreateWidget<UUserWidget>(PlayerOwner, _LoadingScreenClass, TEXT("Main menu"));
			_LoadingScreen->AddToViewport(1);
		}
		else if (_LoadingScreen)
		{
			_LoadingScreen->RemoveFromViewport();
		}
	}
}

void AMainMenuHUD::DisplayLoadingScreen_Multicast_Implementation(bool bShowLoadingScreen)
{
	DisplayLoadingScreen(bShowLoadingScreen);
}
