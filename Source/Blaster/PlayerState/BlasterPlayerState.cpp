// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	float NewScore = GetScore() + ScoreAmount;
	SetScore(NewScore);

	if(GetPawn())
	{
		BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : BlasterCharacter;
		if(BlasterCharacter)
		{
			BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
			if(BlasterController)
			{
				BlasterController->SetHUDScore(NewScore);
			}
		}
	}
}

void ABlasterPlayerState::AddToDefeats(int32 DefeatAmount)
{
	Defeats += DefeatAmount;
	
	if(GetPawn())
	{
		BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : BlasterCharacter;
		if(BlasterCharacter)
		{
			BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
			if(BlasterController)
			{
				BlasterController->SetHUDDefeats(Defeats);
			}
		}
	}	
}


void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Defeats);
	DOREPLIFETIME(ABlasterPlayerState, TeamType);
}

void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	if(GetPawn())
	{
		BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : BlasterCharacter;
		if(BlasterCharacter)
		{
			BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
			if(BlasterController)
			{
				BlasterController->SetHUDScore(GetScore());
			}
		}
	}	
}

void ABlasterPlayerState::OnRep_Defeats()
{
	if(GetPawn())
	{
		BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : BlasterCharacter;
		if(BlasterCharacter)
		{
			BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
			if(BlasterController)
			{
				BlasterController->SetHUDDefeats(Defeats);
			}
		}
	}	
}