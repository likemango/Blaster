// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"

USTRUCT()
struct FHealData
{
	GENERATED_BODY()

	FHealData(): HealRate(0), LastHealTime(0), HealTime(0), HealAmount(0){}

	FHealData(float Amount, float Time);

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

	void Heal(float Amount, float Time);
	
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void HealRampUp(float DeltaTime);
	
private:
	UPROPERTY()
	class ABlasterCharacter* Character;

	UPROPERTY()
	TArray<FHealData> HealDataArray;
	
public:

};
