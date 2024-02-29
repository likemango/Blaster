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
	void SetHUDCarriedAmmo(int32 NewWeaponCarriedAmmo);
	void SetHUDTime(float TimeSeconds);
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	float MatchTime = 120.f;

	uint32 LastSecond = 0;
};
