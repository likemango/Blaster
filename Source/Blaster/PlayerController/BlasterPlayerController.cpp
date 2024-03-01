// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blaster/GameModes/BlasterGameMode.h"
#include "Blaster/HUD/Announcement.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	if(BlasterHUD)
	{
		BlasterHUD->AddAnnouncementOverlay();
	}
}

void ABlasterPlayerController::SetHUDTime()
{
	uint32 CurrentSecond = FMath::FloorToInt(MatchTime - GetServerTime());
	if(CurrentSecond != SecondsLeft)
	{
		// it past more a second
		SetHUDMatchCountDown(MatchTime - GetServerTime());
	}
	SecondsLeft = CurrentSecond;
}

void ABlasterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckTimeSync(DeltaSeconds);
	SetHUDTime();
	PollInit();
}

void ABlasterPlayerController::CheckTimeSync(float TimeDelta)
{
	TimeSyncRunningTime += TimeDelta;
	if(TimeSyncRunningTime > TimeSyncFrequency && IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0;
	}
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if(BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthBar &&
			BlasterHUD->CharacterOverlay->HealthText;
	if(bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"),FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetScore(float NewScore)
{
	BlasterHUD = BlasterHUD ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ScoreText;
	if(bHUDValid)
	{
		FString NewScoreText = FString::Printf(TEXT("%d"),FMath::FloorToInt(NewScore));
		BlasterHUD->CharacterOverlay->ScoreText->SetText(FText::FromString(NewScoreText));
	}
	else
	{
		HUDScores = NewScore;
	}
}

void ABlasterPlayerController::SetDefeats(int32 NewDefeats)
{
	BlasterHUD = BlasterHUD ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->DefeatsText;
	if(bHUDValid)
	{
		FString NewDefeatsText = FString::Printf(TEXT("%d"),NewDefeats);
		BlasterHUD->CharacterOverlay->DefeatsText->SetText(FText::FromString(NewDefeatsText));
	}
	else
	{
		HUDDefeats = NewDefeats;
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 NewWeaponAmmo)
{
	BlasterHUD = BlasterHUD ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoText;
	if(bHUDValid)
	{
		FString NewWeaponAmmoText = FString::Printf(TEXT("%d"),NewWeaponAmmo);
		BlasterHUD->CharacterOverlay->WeaponAmmoText->SetText(FText::FromString(NewWeaponAmmoText));
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 NewWeaponCarriedAmmo)
{
	BlasterHUD = BlasterHUD ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponCarriedAmmoText;
	if(bHUDValid)
	{
		FString NewWeaponCarriedAmmoText = FString::Printf(TEXT("%d"),NewWeaponCarriedAmmo);
		BlasterHUD->CharacterOverlay->WeaponCarriedAmmoText->SetText(FText::FromString(NewWeaponCarriedAmmoText));
	}
}

void ABlasterPlayerController::SetHUDMatchCountDown(float TimeSeconds)
{
	BlasterHUD = BlasterHUD ? BlasterHUD : Cast<ABlasterHUD>(GetHUD());
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchTimeText;
	if(bHUDValid)
	{
		int32 Mines = FMath::FloorToInt(TimeSeconds / 60.f);
		int32 Seconds = TimeSeconds - Mines * 60;
		FString NewMatchTimeText = FString::Printf(TEXT("%02d:%02d"), Mines, Seconds);
		BlasterHUD->CharacterOverlay->MatchTimeText->SetText(FText::FromString(NewMatchTimeText));
	}
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	// this is for each client! !HasAuthority() is more than that, including proxy!
	if(IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
}

float ABlasterPlayerController::GetServerTime() const
{
	if(HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	else
	{
		return GetWorld()->GetTimeSeconds() + ClientServerTimeDelta;
	}
}

// execute on server
void ABlasterPlayerController::ServerRequestServerTime_Implementation(float ClientRequestTime)
{
	float ServerCurrentTime = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(ClientRequestTime, ServerCurrentTime);
}

// execute on client
void ABlasterPlayerController::ClientReportServerTime_Implementation(float ClientRequestTime, float ServerSendTime)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - ClientRequestTime;
	float CurrentServerTime = RoundTripTime * 0.5f + ServerSendTime;
	ClientServerTimeDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		BlasterHUD->AddCharacterOverlay();
		if(BlasterHUD->AnnouncementOverlay)
		{
			BlasterHUD->AnnouncementOverlay->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::SetMatchState(FName NewState)
{
	MatchState = NewState;

	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void ABlasterPlayerController::PollInit()
{
	if(!CharacterOverlay)
	{
		if(GetHUD())
		{
			CharacterOverlay = Cast<ABlasterHUD>(GetHUD())->CharacterOverlay;
			if(CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetScore(HUDScores);
				SetDefeats(HUDDefeats);
				bInitializeCharacterOverlay = true;
			}
		}
	}
}
