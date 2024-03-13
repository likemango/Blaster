// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

UCLASS()
class BLASTER_API AHealthPickup : public APickup
{
	GENERATED_BODY()

public:
	AHealthPickup();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverLappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
private:
	UPROPERTY(EditAnywhere)
	float HealAmount;

	UPROPERTY(EditAnywhere)
	float HealTime;
	
	UPROPERTY(EditAnywhere)
	class UNiagaraComponent* PickupEffectComp;
	
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffect;
	
public:

	
};
