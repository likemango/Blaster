// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FName GetAttachSocket();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Interact(APawn* Pawn);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsPickup();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnPickup(USceneComponent* Component, FName Socket);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnDrop();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanBePickedUp();
};
