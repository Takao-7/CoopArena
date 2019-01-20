// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Humanoid.h"
#include "Bot.generated.h"


UCLASS()
class COOPARENA_API ABot : public AHumanoid
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bot character")
	TArray<AActor*> _PatrolPoints;

	/* Extra spread, in degrees, that will be added when we are firing any weapon. A negative value will reduce the effective spread! */
	UPROPERTY(EditDefaultsOnly, Category = "Bot character")
	float _ExtraHorizontalSpread = 3.0f;

	/* Extra spread, in degrees, that will be added when we are firing any weapon. A negative value will reduce the effective spread! */
	UPROPERTY(EditDefaultsOnly, Category = "Bot character")
	float _ExtraVerticalSpread = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Bot character", meta = (ClampMin = 0.0f))
	float _PointsPerKill = 10.0f;

public:
	/**
	* Returns the additional spread, in degree. 
	* X = Horizontal
	* Y = Vertical
	*/
	UFUNCTION(BlueprintPure, Category = "Bot character")
	FVector2D GetAdditionalSpread() const { return FVector2D(_ExtraHorizontalSpread, _ExtraVerticalSpread); };

	UFUNCTION(BlueprintPure, Category = "Bot character")
	float GetPointsPerKill() const { return _PointsPerKill; };
};