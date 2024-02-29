﻿// Fill out your copyright notice in the Description page of Project Settings.

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
	void SetHUDMatchCountDown(float TimeSeconds);
	virtual void ReceivedPlayer() override;
	float GetServerTime() const;
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	virtual void Tick(float DeltaSeconds) override;
	
private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	float MatchTime = 120.f;

	uint32 SecondsLeft = 0;

	UFUNCTION(Server,Reliable)
	void ServerRequestServerTime(float ClientRequestTime); // client send request to server
	UFUNCTION(Client,Reliable)
	void ClientReportServerTime(float ClientRequestTime, float ServerSendTime); // server receive client request, and send back its current time

	UPROPERTY()
	float ClientServerTimeDelta = 0; // the different between client and server: serverTime - clientTime

	UPROPERTY(EditAnywhere, Category=Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0;
	void CheckTimeSync(float TimeDelta);
};
