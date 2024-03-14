// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


FHealData::FHealData(float Amount, float Time, bool HealHealth, bool HealShield)
{
	HealAmount = Amount;
	HealTime = Time;
	bHealHealth = HealHealth;
	bHealShield = HealShield;

	HealRate = HealAmount / HealTime;
	LastHealTime = HealTime;
}

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void UBuffComponent::HealBuff(float Amount, float Time)
{
	FHealData HealData(Amount,Time, true, false);
	HealDataArray.Add(HealData);
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if(!Character || Character->IsEliminated())
	{
		HealDataArray.Empty();
	}
	if(HealDataArray.Num() > 0)
	{
		float HealDeltaThisFrame = 0.f;
		for (auto It = HealDataArray.CreateIterator(); It; ++It)
		{
			FHealData& HealData = *It;
			if(!HealData.bHealHealth || HealData.LastHealTime <= 0)
			{
				continue;
			}
			HealDeltaThisFrame += HealData.HealRate;
			HealData.LastHealTime -= DeltaTime;
			if(HealData.LastHealTime <= 0)
			{
				It.RemoveCurrent();
			}
		}
		float NewHealth = FMath::Clamp(Character->GetHealth() + HealDeltaThisFrame*DeltaTime, 0, Character->GetMaxHealth());
		Character->SetHealth(NewHealth);
		Character->UpdateHealthHUD();
	}	
}

void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	FHealData HealData(ShieldAmount,ReplenishTime, false, true);
	HealDataArray.Add(HealData);
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if(!Character || Character->IsEliminated())
	{
		HealDataArray.Empty();
	}
	if(HealDataArray.Num() > 0)
	{
		float HealDeltaThisFrame = 0.f;
		for (auto It = HealDataArray.CreateIterator(); It; ++It)
		{
			FHealData& HealData = *It;
			if(!HealData.bHealShield || HealData.LastHealTime <= 0)
			{
				continue;
			}
			HealDeltaThisFrame += HealData.HealRate;
			HealData.LastHealTime -= DeltaTime;
			if(HealData.LastHealTime <= 0)
			{
				It.RemoveCurrent();
			}
		}
		float NewShield = FMath::Clamp(Character->GetShield() + HealDeltaThisFrame*DeltaTime, 0, Character->GetMaxShield());
		Character->SetShield(NewShield);
		Character->UpdateHUDShield();
	}	
}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	WalkInitialSpeed = BaseSpeed;
	CrouchInitialSpeed = CrouchSpeed;
}

void UBuffComponent::SpeedBuff(float WalkSpeed, float CrouchSpeed, float BuffTime)
{
	if(GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			SpeedBuffTimerHandle,
			this,
			&ThisClass::OnSpeedBuffTimerEnd,
			BuffTime
		);
	}
	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	}
	MulticastChangeSpeed(WalkSpeed, CrouchSpeed);
}

void UBuffComponent::OnSpeedBuffTimerEnd()
{
	// reset speed
	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = WalkInitialSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchInitialSpeed;
	}
	MulticastChangeSpeed(WalkInitialSpeed, CrouchInitialSpeed);
}

void UBuffComponent::MulticastChangeSpeed_Implementation(float Walk, float Crouch)
{
	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = Walk;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = Crouch;
	}
}

void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
}

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if (Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&UBuffComponent::ResetJump,
		BuffTime
	);

	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
	}
	MulticastJumpBuff(BuffJumpVelocity);
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}

void UBuffComponent::ResetJump()
{
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	}
	MulticastJumpBuff(InitialJumpVelocity);
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}


