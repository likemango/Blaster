// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "UObject/Interface.h"
#include "WeaponInterface.generated.h"



// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UWeaponInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class BLASTER_API IWeaponInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void Fire();
	virtual void StopFire();
	virtual bool CanFire();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Reload();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ReloadComplete();

	virtual void CycleFireMode();
	virtual EBlasterWeaponPriorityType GetWeaponPriorityType();
	virtual void ActionFinishedCycling();
};
