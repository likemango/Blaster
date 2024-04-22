// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamGameMode.h"

#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"

void ATeamGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if(NewPlayer && BlasterGameState)
	{
		if(ABlasterPlayerState* NewPlayerState = NewPlayer->GetPlayerState<ABlasterPlayerState>())
		{
			if(NewPlayerState && NewPlayerState->GetTeamType() == ETeamTypes::ET_NoTeam)
			{
				if(BlasterGameState->RedTeam.Num() >= BlasterGameState->BlueTeam.Num())
				{
					BlasterGameState->BlueTeam.AddUnique(NewPlayerState);
					NewPlayerState->SetTeamType(ETeamTypes::ET_BlueTeam);
				}
				else
				{
					BlasterGameState->RedTeam.Emplace(NewPlayerState);
					NewPlayerState->SetTeamType(ETeamTypes::ET_RedTeam);
				}
			}
		}
	}
}

void ATeamGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if(Exiting)
	{
		if(ABlasterPlayerState* ExitingPlayerState = Exiting->GetPlayerState<ABlasterPlayerState>())
		{
			if(ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>())
			{
				if(BlasterGameState->RedTeam.Contains(ExitingPlayerState))
				{
					BlasterGameState->RedTeam.Remove(ExitingPlayerState);
				}
				else
				{
					BlasterGameState->BlueTeam.Emplace(ExitingPlayerState);
				}
			}
		}
	}
}

float ATeamGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	ABlasterPlayerState* AttackerPState = Attacker->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState* VictimPState = Victim->GetPlayerState<ABlasterPlayerState>();
	if (AttackerPState == nullptr || VictimPState == nullptr) return BaseDamage;
	if (VictimPState == AttackerPState)
	{
		return BaseDamage;
	}
	if (AttackerPState->GetTeamType() == VictimPState->GetTeamType())
	{
		return 0.f;
	}
	return BaseDamage;
}

void ATeamGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	if(ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>())
	{
		for(APlayerState* EachPlayerState : BlasterGameState->PlayerArray)
		{
			if(ABlasterPlayerState* EachBlasterPlayerState = Cast<ABlasterPlayerState>(EachPlayerState))
			{
				if(EachBlasterPlayerState && EachBlasterPlayerState->GetTeamType() == ETeamTypes::ET_NoTeam)
				{
					if(BlasterGameState->RedTeam.Num() >= BlasterGameState->BlueTeam.Num())
					{
						BlasterGameState->BlueTeam.Emplace(EachBlasterPlayerState);
						EachBlasterPlayerState->SetTeamType(ETeamTypes::ET_BlueTeam);
					}
					else
					{
						BlasterGameState->RedTeam.Emplace(EachBlasterPlayerState);
						EachBlasterPlayerState->SetTeamType(ETeamTypes::ET_RedTeam);
					}
				}
			}
		}
	}
}
