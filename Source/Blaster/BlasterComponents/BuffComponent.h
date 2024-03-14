// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"

USTRUCT()
struct FHealData
{
	GENERATED_BODY()

	FHealData(): bHealHealth(false), bHealShield(false), HealRate(0), LastHealTime(0), HealTime(0), HealAmount(0)
	{
	}

	FHealData(float Amount, float Time, bool HealHealth, bool HealShield);

	UPROPERTY()
	bool bHealHealth;

	UPROPERTY()
	bool bHealShield;
	
	UPROPERTY()
	float HealRate;

	UPROPERTY()
	float LastHealTime;
	
private:
	UPROPERTY()
	float HealTime;

	UPROPERTY()
	float HealAmount;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()
	friend class ABlasterCharacter;
	
public:
	UBuffComponent();

	void HealBuff(float Amount, float Time);
	void SpeedBuff(float WalkSpeed, float CrouchSpeed, float BuffTime);
	void BuffJump(float BuffJumpVelocity, float BuffTime);
	void ReplenishShield(float ShieldAmount, float ReplenishTime);
	void SetInitialJumpVelocity(float Velocity);
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);
protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void HealRampUp(float DeltaTime);
	void ShieldRampUp(float DeltaTime);
	
private:
	
	UPROPERTY()
	class ABlasterCharacter* Character;

	// Heal Buff
	UPROPERTY()
	TArray<FHealData> HealDataArray;

	// Speed Buff
	FTimerHandle SpeedBuffTimerHandle;
	void OnSpeedBuffTimerEnd();
	UPROPERTY()
	float WalkInitialSpeed;
	UPROPERTY()
	float CrouchInitialSpeed;
	UFUNCTION(NetMulticast, Reliable)
	void MulticastChangeSpeed(float Walk, float Crouch);
	
	/** 
	* Jump buff
	*/
	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpVelocity;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);

	/** 
	* Shield buff
	*/
	bool bReplenishingShield = false;
	float ShieldReplenishRate = 0.f;
	float ShieldReplenishAmount = 0.f;
};
