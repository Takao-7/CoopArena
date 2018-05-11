// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Tool.generated.h"


class AActor;


/**
 * Interface for objects that can be used while being held in hands, for example weapons.
 * Classes that implement this interface should also implement the IPickUp interface, because you need to pick up an object first,
 * before you can use it.
 */
UINTERFACE(BlueprintType)
class COOPARENA_API UTool : public UInterface
{
	GENERATED_BODY()
	
};


class COOPARENA_API ITool
{
	GENERATED_BODY()

public:
	/**
	 * Use this tool's primary function (left mouse button by default)
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = Tool)
	void UseTool();

	/**
	* Use this tool's secondary function (right mouse button by default)
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = Tool)
	void UseToolSecondary();

	/**
	 * Reload the tool.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = Tool)
	void ReloadTool();

	/**
	 * Stop this tool's functionality.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = Tool)
	void StopUsingTool();

	/**
	 * Returns the cooldown time, in seconds, between the tool's uses,
	 * when UseTool() is called each frame (LMB is held down). 
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = Tool)
	float GetCooldownTime() const;

	/**
	* Returns the cooldown time, in seconds, between the tool's uses,
	* when UseToolSecondary() is called each frame (RMB is held down).
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = Tool)
	float GetCooldownTimeSecondary() const;
};