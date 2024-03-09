// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	AProjectile();

protected:
	virtual void BeginPlay() override;
	//for replicated actor, destroyed function gonna be called in all clients!
	virtual void Destroyed() override;
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);
	
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* BoxComponent;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ProjectileTrace;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* HitParticle;

	UPROPERTY(EditAnywhere)
	class USoundCue* HitSoundCue;

	UPROPERTY()
	UParticleSystemComponent* ParticleSystemComponent;

	UPROPERTY(EditAnywhere,Category=Damage)
	float DamageValue = 20.f;
};
