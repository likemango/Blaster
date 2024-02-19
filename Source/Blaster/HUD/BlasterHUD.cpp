// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y/ 2.f);
		if (HUDPackage.CrosshairsCenter)
		{
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter);
		}
		if (HUDPackage.CrosshairsRight)
		{
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter);
		}
		if (HUDPackage.CrosshairsTop)
		{
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter);
		}
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* CrosshairTexture, const FVector2D& ViewPortCenter)
{
	const uint32 SizeX = CrosshairTexture->GetSizeX();
	const uint32 SizeY = CrosshairTexture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewPortCenter.X - SizeX / 2.f,
		ViewPortCenter.Y - SizeY / 2.f
	);
	DrawTexture(CrosshairTexture, TextureDrawPoint.X, TextureDrawPoint.Y,
		SizeX, SizeY,
		0, 0,
		1, 1,
		FColor::White);
}
