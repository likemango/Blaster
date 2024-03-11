// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;
protected:
	virtual FVector TraceEndWithScatter(const FVector& FireStart, const FVector& HitTarget);
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticle;
	
	UPROPERTY(EditAnywhere)
	USoundCue* HitSound;

	UPROPERTY(EditAnywhere)
	float DamageValue;
	
private:

	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticle;
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;

	/*
	 * Fire Scatter
	 */
	UPROPERTY(EditAnywhere, Category="WeaponScatter")
	float DistanceScatterSphere = 800.f;
	UPROPERTY(EditAnywhere, Category="WeaponScatter")
	float ScatterSphereRadius = 75.f;
	UPROPERTY(EditAnywhere, Category="WeaponScatter")
	bool bUseScatter = false;
	
};
