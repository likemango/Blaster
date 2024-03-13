// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

#include "Blaster/Character/BlasterCharacter.h"


FHealData::FHealData(float Amount, float Time)
{
	HealAmount = Amount;
	HealTime = Time;

	HealRate = HealAmount / HealTime;
	LastHealTime = HealTime;
}

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
}

void UBuffComponent::Heal(float Amount, float Time)
{
	FHealData HealData(Amount,Time);
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
			HealDeltaThisFrame += HealData.HealRate;
			HealData.LastHealTime -= DeltaTime;
			if(HealData.LastHealTime <= 0)
			{
				It.RemoveCurrent();
			}
		}
		if(Character)
		{
			float NewHealth = FMath::Clamp(Character->GetHealth() + HealDeltaThisFrame*DeltaTime, 0, Character->GetMaxHealth());
			Character->SetHealth(NewHealth);
			Character->UpdateHealthHUD();

			if(Character->GetHealth() >= Character->GetMaxHealth())
			{
				HealDataArray.Empty();
			}
		}
	}	
	
}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
}


