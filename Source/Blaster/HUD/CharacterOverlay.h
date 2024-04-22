// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* HealthText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RedTeamScore;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BlueTeamScore;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreSpacerText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ScoreText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* DefeatsText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* WeaponAmmoText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* WeaponCarriedAmmoText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* MatchTimeText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* GrenadeAmmoText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PingText;

	UPROPERTY(meta = (BindWidget))
	UImage* HighPingImage;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HighPingAnimation;
};
