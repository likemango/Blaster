// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

#include "Announcement.h"
#include "CharacterOverlay.h"
#include "Blueprint/UserWidget.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ABlasterHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if(PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::AddAnnouncementOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if(PlayerController && AnnouncementOverlayClass)
	{
		AnnouncementOverlay = CreateWidget<UAnnouncement>(PlayerController, AnnouncementOverlayClass);
		AnnouncementOverlay->AddToViewport();
	}
}


void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y/ 2.f);
		
		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
		if (HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsTop)
		{
			// screen upper-left is original
			// negative to move it up 
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* CrosshairTexture, const FVector2D& ViewPortCenter, const FVector2D& Spread, const FLinearColor& ChooseCrosshairColor)
{
	const uint32 SizeX = CrosshairTexture->GetSizeX();
	const uint32 SizeY = CrosshairTexture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewPortCenter.X - SizeX / 2.f + Spread.X,
		ViewPortCenter.Y - SizeY / 2.f + Spread.Y
	);
	DrawTexture(CrosshairTexture, TextureDrawPoint.X, TextureDrawPoint.Y,
		SizeX, SizeY,
		0, 0,
		1, 1,
		ChooseCrosshairColor);
}
