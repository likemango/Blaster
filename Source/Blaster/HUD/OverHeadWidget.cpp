// Fill out your copyright notice in the Description page of Project Settings.


#include "OverHeadWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverHeadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

FString UOverHeadWidget::GetRemoteNetRole(APawn* InPawn)
{
	ENetRole RemoteRole = InPawn->GetRemoteRole();
	FString Role;
	switch (RemoteRole)
	{
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("Autonomous Proxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("Simulated Proxy");
		break;
	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	default:
			break;
	}
	FString RemoteRoleString = FString::Printf(TEXT("Remote Role: %s"), *Role);
	return RemoteRoleString;
}

FString UOverHeadWidget::GetLocalNetRole(APawn* InPawn)
{
	ENetRole LocalRole = InPawn->GetLocalRole();
	FString Role;
	switch (LocalRole)
	{
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("Autonomous Proxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("Simulated Proxy");
		break;
	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	default:
		break;
	}
	FString LocalRoleString = FString::Printf(TEXT("Local Role: %s"), *Role);
	return LocalRoleString;
}

void UOverHeadWidget::ShowNetRole(APawn* InPawn)
{
	if(!InPawn)
		return;

	FString RemoteRole = GetRemoteNetRole(InPawn);
	FString LocalRole = GetLocalNetRole(InPawn);
	FString Role = RemoteRole.Append(TEXT("\n")).Append(LocalRole);
	if(DisplayText)
	{
		DisplayText->SetText(FText::FromString(Role));
	}
}

void UOverHeadWidget::ShowPlayerName(APawn* InPawn)
{
	APlayerState* PlayerState = InPawn->GetPlayerState();
	FString ShowPlayNameText = FString(TEXT("PlayerName: "));
	if(PlayerState)
	{
		FString PlayerName = PlayerState->GetPlayerName();
		ShowPlayNameText = FString(TEXT("PlayerName: ")).Append(PlayerName);
	}
	else
	{
		FString PlayerName = FString(TEXT("No Ready!"));
		ShowPlayNameText = FString(TEXT("PlayerName: ")).Append(PlayerName);
	}
	if(DisplayText)
	{
		DisplayText->SetText(FText::FromString(ShowPlayNameText));
	}
}

