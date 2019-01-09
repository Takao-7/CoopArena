// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MyHUD.generated.h"


UENUM(BlueprintType)
enum class EHUDState : uint8
{
	MainMenu,
	Playing,
	Spectating,
	MatchEnd
};


UCLASS()
class COOPARENA_API AMyHUD : public AHUD
{
	GENERATED_BODY()


public:
	UFUNCTION(BlueprintNativeEvent, Category = "HUD")
	void SetState(EHUDState  State);
	void SetState_Implementation(EHUDState  State);

	UFUNCTION(BlueprintPure, Category = "HUD")
	EHUDState GetCurrentState() const { return _HUDState; };

private:
	EHUDState _HUDState;
};
