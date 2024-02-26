// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"

#include "Blaster/Character/BlasterCharacter.h"


// Sets default values
ABlasterGameMode::ABlasterGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController,
	ABlasterPlayerController* AttackerController)
{
	if(ElimmedCharacter)
	{
		ElimmedCharacter->Eliminate();
	}
}
