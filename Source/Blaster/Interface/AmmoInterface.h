// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AmmoInterface.generated.h"

class USKGPDAProjectile;
// This class does not need to be modified.
UINTERFACE()
class UAmmoInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class BLASTER_API IAmmoInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent)
	USKGPDAProjectile* Demo_GetProjectileType();
	UFUNCTION(BlueprintNativeEvent)
	int Demo_GetMagazineCapacity();
};
