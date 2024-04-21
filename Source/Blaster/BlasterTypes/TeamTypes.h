#pragma once

UENUM()
enum class ETeamTypes : uint8
{
	ET_RedTeam UMETA(DisplayName="RedTeam"),
	ET_BlueTeam UMETA(DisplayName="BlueTeam"),
	ET_NoTeam UMETA(DisplayName="NoTeam"),

	ET_MAX UMETA(DisplayName="MAX")
};