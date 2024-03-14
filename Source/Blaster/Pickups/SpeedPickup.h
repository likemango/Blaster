// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "SpeedPickup.generated.h"

UCLASS()
class BLASTER_API ASpeedPickup : public APickup
{
	GENERATED_BODY()

public:

protected:
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverLappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
private:
	UPROPERTY(EditAnywhere)
	float WalkBuffSpeed = 1600.f;

	UPROPERTY(EditAnywhere)
	float CrouchBuffSpeed = 1000.f;

	UPROPERTY(EditAnywhere)
	float BuffTime = 3.f;
};
