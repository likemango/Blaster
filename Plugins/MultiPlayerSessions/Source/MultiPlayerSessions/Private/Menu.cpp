// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"

#include "MultiPlayerSessionSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Components/Button.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnection, FString TypeOfMath, FString PathOfLobby)
{
	LobbyPath = FString::Printf(TEXT("%s?Listen"), *PathOfLobby);
	NumPublicConnections = NumberOfPublicConnection;
	MatchType = TypeOfMath;
	
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if(World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if(PlayerController)
		{
			FInputModeUIOnly InputModeUIOnly;
			InputModeUIOnly.SetWidgetToFocus(TakeWidget());
			InputModeUIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeUIOnly);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	MultiPlayerSessionSubsystem = GetGameInstance()->GetSubsystem<UMultiPlayerSessionSubsystem>();
	if(MultiPlayerSessionSubsystem)
	{
		MultiPlayerSessionSubsystem->MultiPlayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiPlayerSessionSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiPlayerSessionSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiPlayerSessionSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiPlayerSessionSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

bool UMenu::Initialize()
{
	if(!Super::Initialize())
	{
		return false;
	}
	if(HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if(JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}
	return true;
}

void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 15.0f, FColor::Blue, TEXT("Session Created Successfully!"));
		}
		
		// server travel to lobby
		if(GetWorld())
		{
			GetWorld()->ServerTravel(LobbyPath);
		}
	}
	else
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 15.0f, FColor::Red, TEXT("Session Created Failed!"));
		}
		HostButton->SetIsEnabled(true);
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (MultiPlayerSessionSubsystem == nullptr)
	{
		return;
	}
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 15.0f, FColor::Blue, FString::Printf(TEXT("Session Found Nums: %d"), SessionResults.Num()));
	}
	for (auto Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		if (SettingsValue == MatchType)
		{
			MultiPlayerSessionSubsystem->JoinSession(Result);
			return;
		}
	}
	if(!bWasSuccessful || SessionResults.Num() == 0)
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 15.0f, FColor::Red, TEXT("Session Find Failed!"));
		}
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
	if(Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	if(MultiPlayerSessionSubsystem)
	{
		// create session
		MultiPlayerSessionSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if (MultiPlayerSessionSubsystem)
	{
		// after found session successful, gonna join the session.
		MultiPlayerSessionSubsystem->FindSessions(10000);
	}
}

void UMenu::MenuTearDown()
{
	// remove it.
	RemoveFromParent();
	
	if(GetWorld())
	{
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		if(PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}
