﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
protected:
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	
private:

	UPROPERTY(VisibleAnywhere)
	class URocketProjectileMovement* RocketProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileLoopSound;
	UPROPERTY()
	UAudioComponent* ProjectileLSoundCurLoopComponent;
	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;
	
};
