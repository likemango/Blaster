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
	virtual void ReceivedPlayer() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float NewScore);
	void SetHUDDefeats(int32 NewDefeats);
	void SetHUDWeaponAmmo(int32 NewWeaponAmmo);
	void SetHUDCarriedAmmo(int32 NewWeaponCarriedAmmo);
	void SetHUDGrenadeAmmo(int32 NewGrenadeAmmo);
	void SetHUDMatchCountDown(float TimeSeconds);
	void SetHUDAnnouncementCountdown(float TimeSeconds);
	void SetHUDShield(float NewShield, float MaxShield);
	float GetServerTime() const;
	void HandleMatchHasStarted();
	void HandleMatchCoolDown();
	void OnMatchStateSet(FName NewState);
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	virtual void Tick(float DeltaSeconds) override;
	
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();
	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float StartingTime, float TimeOfCoolDown);
	
private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CoolDownTime = 0.f;
	
	uint32 CountdownInt = 0;

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

	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

	void PollInit();

	float HUDHealth;
	bool bInitializeHealth = false;
	float HUDMaxHealth;
	float HUDScore;
	bool bInitializeScore = false;
	int32 HUDDefeats;
	bool bInitializeDefeats = false;
	int32 HUDGrenades;
	bool bInitializeGrenades = false;
	float HUDShield;
	bool bInitializeShield = false;
	float HUDMaxShield;
};
