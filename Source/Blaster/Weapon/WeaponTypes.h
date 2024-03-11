#pragma once

#define TRACE_LINE_LENGTH 80000.0f;

UENUM(BlueprintType)
enum class EBlasterWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),
	EWT_RocketLauncher UMETA(DisplayName = "RocketLauncher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_SMG UMETA(DisplayName = "SubmachineGun"),
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	EWT_MAX UMETA(DisplayName = "DefaultMax")
};