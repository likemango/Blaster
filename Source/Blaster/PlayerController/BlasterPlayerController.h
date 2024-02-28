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
	void SetDefeats(int32 NewDefeats);
	void SetHUDWeaponAmmo(int32 NewWeaponAmmo);
	void SetHUDWeaponCarriedAmmo(int32 NewWeaponCarriedAmmo);
	
protected:
	void BeginPlay() override;

private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

};
