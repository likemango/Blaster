// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

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
				BlasterController->SetScore(GetScore());
			}
		}
	}	
}

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
				BlasterController->SetScore(NewScore);
			}
		}
	}
}
