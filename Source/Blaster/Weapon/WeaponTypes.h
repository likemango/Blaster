#pragma once

UENUM(BlueprintType)
enum class EBlasterWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),
	EWT_RocketLauncher UMETA(DisplayName = "RocketLauncher"),
	
	EWT_MAX UMETA(DisplayName = "DefaultMax")
};