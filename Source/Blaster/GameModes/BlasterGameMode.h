// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

namespace MatchState
{
	extern BLASTER_API FName CoolDown;
}

UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABlasterGameMode();
	virtual void PlayerEliminated(class ABlasterCharacter* EliminatedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
	virtual void RespawnCharacter(ACharacter* EliminatedCharacter, AController* EliminatedController);
	virtual void OnMatchStateSet() override;
	void PlayerLeftGame(class ABlasterPlayerState* LeavingPlayerState);
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;
	UPROPERTY(EditDefaultsOnly)
	float CoolDownTime = 10.f;
	
	float LevelBeginTime = 0.f;
	FORCEINLINE float GetCountDownTime() const { return CountDownTime;}

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
private:
	float CountDownTime = 0.f;
};
