#pragma once

UENUM()
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_ThrowGrenade UMETA(DisplayName = "ThrowGrenade"),
	
	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};


