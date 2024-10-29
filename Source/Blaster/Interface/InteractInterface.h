// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInteractInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class BLASTER_API IInteractInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	FName GetAttachSocket();
	
	UFUNCTION(BlueprintNativeEvent)
	void Interact(APawn* Pawn);
	
	UFUNCTION(BlueprintNativeEvent)
	bool IsPickup();
	
	UFUNCTION(BlueprintNativeEvent)
	void OnPickup(USceneComponent* Component, FName Socket);
	
	UFUNCTION(BlueprintNativeEvent)
	void OnDrop();
	
	UFUNCTION(BlueprintNativeEvent)
	bool CanBePickedUp();
};
