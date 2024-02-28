// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
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
