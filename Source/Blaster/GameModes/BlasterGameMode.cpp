// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

namespace MatchState
{
	FName CoolDown = FName(TEXT("CoolDown"));
}

// Sets default values
ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();
	LevelBeginTime = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(MatchState == MatchState::WaitingToStart)
	{
		CountDownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelBeginTime;
		if(CountDownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if(MatchState == MatchState::InProgress)
	{
		CountDownTime = WarmupTime + MatchTime -  GetWorld()->GetTimeSeconds() + LevelBeginTime;
		if(CountDownTime <= 0.f)
		{
			SetMatchState(MatchState::CoolDown);
		}
	}
	else if(MatchState == MatchState::CoolDown)
	{
		CountDownTime = CoolDownTime + WarmupTime + MatchTime -  GetWorld()->GetTimeSeconds() + LevelBeginTime;
		if(CountDownTime <= 0.f)
		{
			// from GameMode.h
			RestartGame();
		}
	}
}

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterPlayerState* AttackerPlayerState = AttackerController->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState* VictimPlayerState = VictimController->GetPlayerState<ABlasterPlayerState>();
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	
	if(AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlasterGameState)
	{
		TArray<ABlasterPlayerState*> OldTopScores;
		for(ABlasterPlayerState* PlayerState : BlasterGameState->TopScoringPlayers)
		{
			OldTopScores.Emplace(PlayerState);
		}
		AttackerPlayerState->AddToScore(1.f);
		BlasterGameState->UpdateTopScores(AttackerPlayerState);
		if(BlasterGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			if(ABlasterCharacter* Character = Cast<ABlasterCharacter>(AttackerPlayerState->GetPawn()))
			{
				Character->OnLeadTheCrown();
			}
		}
		for(ABlasterPlayerState* PlayerState : OldTopScores)
		{
			if(!BlasterGameState->TopScoringPlayers.Contains(PlayerState))
			{
				if(ABlasterCharacter* Character = Cast<ABlasterCharacter>(PlayerState->GetPawn()))
				{
					Character->OnLoseTheCrown();
				}
			}
		}
	}
	if(VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	
	if(EliminatedCharacter)
	{
		EliminatedCharacter->Eliminate(false);
	}
}

void ABlasterGameMode::RespawnCharacter(ACharacter* EliminatedCharacter,AController* EliminatedController)
{
	if(EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}
	if(EliminatedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
		uint32 RandomIndex = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(EliminatedController, PlayerStarts[RandomIndex]);
	}
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(*It);
		if(BlasterPlayerController)
		{
			BlasterPlayerController->OnMatchStateSet(MatchState);
		}
	}
}

void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* LeavingPlayerState)
{
	if(LeavingPlayerState)
	{
		ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(GameState);
		if(BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(LeavingPlayerState))
		{
			BlasterGameState->TopScoringPlayers.Remove(LeavingPlayerState);
		}
		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(LeavingPlayerState->GetPawn());
		if(BlasterCharacter)
		{
			BlasterCharacter->Eliminate(true);
		}
	}
}


