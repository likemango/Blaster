// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "AmmoPickup.generated.h"

UCLASS()
class BLASTER_API AAmmoPickup : public APickup
{
	GENERATED_BODY()

public:
	AAmmoPickup();
	virtual void Tick(float DeltaSeconds) override;
protected:
	virtual void BeginPlay() override;
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverLappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
public:

private:
	UPROPERTY(EditAnywhere)
	int32 AmmoAmount = 20;
	UPROPERTY(EditAnywhere)
	EBlasterWeaponType AmmoWeaponType = EBlasterWeaponType::EWT_MAX;
	UPROPERTY(EditAnywhere)
	float RotateRate = 45.f;
};
