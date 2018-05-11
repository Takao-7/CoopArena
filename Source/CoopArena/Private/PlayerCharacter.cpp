// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"


APlayerCharacter::APlayerCharacter()
{
	
}


FVector APlayerCharacter::GetCameraLocation() const
{
	UCameraComponent* camera = Cast<UCameraComponent>(GetComponentByClass(UCameraComponent::StaticClass()));
	if (camera)
	{
		return camera->GetComponentLocation();
	}
	else
	{
		return FVector::ZeroVector;
	}
}
