// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Blaster/BlasterTypes/TeamTypes.h"
#include "BlasterGameState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateTopScores(class ABlasterPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> TopScoringPlayers;

	/*
	 * Teams
	 */
	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> RedTeam;
	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> BlueTeam;
	
	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	UFUNCTION()
	void OnRep_RedTeamScore();

	UFUNCTION()
	void OnRep_BlueTeamScore();

private:
	float TopScore = 0.f;
};
