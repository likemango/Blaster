// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void OnPossess(APawn* InPawn) override;
	
	void SetHUDHealth(float Health, float MaxHealth);
	void SetScore(float NewScore);
	
protected:
	void BeginPlay() override;

private:
	class ABlasterHUD* BlasterHUD;

};
