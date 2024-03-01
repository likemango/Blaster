// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

class UAnnouncement;
class UCharacterOverlay;

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	float CrosshairSpread;
	FLinearColor CrosshairColor;
};

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category="Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;
	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;
	void AddCharacterOverlay();

	UPROPERTY(EditAnywhere, Category="Player Stats")
	TSubclassOf<class UUserWidget> AnnouncementOverlayClass;
	UPROPERTY()
	UAnnouncement* AnnouncementOverlay;
	void AddAnnouncementOverlay();
	
protected:
	void BeginPlay() override;
	
private:
	FHUDPackage HUDPackage;
	void DrawCrosshair(UTexture2D* CrosshairTexture, const FVector2D& ViewPortCenter, const FVector2D& Spread, const FLinearColor& ChooseCrosshairColor);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;
public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) {HUDPackage = Package;}
};
