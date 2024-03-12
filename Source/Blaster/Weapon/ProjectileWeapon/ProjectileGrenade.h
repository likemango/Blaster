// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Weapon/ProjectileWeapon/Projectile/Projectile.h"
#include "ProjectileGrenade.generated.h"

UCLASS()
class BLASTER_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileGrenade();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* BoundSound;

private:
	UFUNCTION()
	void OnBounce( const FHitResult& ImpactResult, const FVector& ImpactVelocity);
};
