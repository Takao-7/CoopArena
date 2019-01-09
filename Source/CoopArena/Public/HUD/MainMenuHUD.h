// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyHUD.h"
#include "MainMenuHUD.generated.h"


class UUserWidget;


UCLASS()
class COOPARENA_API AMainMenuHUD : public AMyHUD
{
	GENERATED_BODY()
	
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Main menu")
	TSubclassOf<UUserWidget> _LoadingScreenClass;

private:
	UUserWidget* _LoadingScreen;
	
public:
	/* Shows or hides the loading screen. */
	UFUNCTION(BlueprintCallable, Category = "Main menu")
	void DisplayLoadingScreen(bool bShowLoadingScreen);

	UFUNCTION(NetMulticast, Reliable, Category = "Main menu")
	void DisplayLoadingScreen_Multicast(bool bShowLoadingScreen);
};
