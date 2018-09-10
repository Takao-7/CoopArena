// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPhysicalMaterial.h"
#include "CoopArena.h"
#include "Engine/World.h"


UMyPhysicalMaterial::UMyPhysicalMaterial()
{
	RHA_Kinetic = 1.0f;
	RHA_Explosive = 1.0f;
	Thickness = 0.0f;
	DamageMod = 1.0f;
}


float UMyPhysicalMaterial::GetAmourThickness(FVector ImpactPoint, FVector Direction, float RayLength /*= 500.0f*/)
{
	if (Thickness != 0.0f)
	{
		return Thickness;
	}
	
	TArray<FHitResult> hitResults;
	FVector endPoint;
	endPoint = ImpactPoint + (Direction * RayLength);
	GetWorld()->LineTraceMultiByChannel(hitResults, ImpactPoint, endPoint, ECC_ProjectilePenetration);
	
	AActor* actor = hitResults[0].GetActor();
	FVector exitPoint;
	bool bCalcThickness = false;

	for (int32 i = 1; i < hitResults.Num(); i++)
	{
		if (hitResults[i].GetActor() == actor)
		{
			exitPoint = hitResults[i].Location;
			bCalcThickness = true;
			break;
		}
	}

	float effectiveThickness = 0.0f;
	if (bCalcThickness)
	{
		effectiveThickness = (exitPoint - ImpactPoint).Size();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Amour thichness couldn't be calculated."));
		effectiveThickness = RayLength;
	}

	return effectiveThickness;
}
