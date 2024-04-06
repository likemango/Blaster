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

	/*
	 *	Used with server-side rewind
	 */
	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000.f;

protected:
	virtual void BeginPlay() override;
	//for replicated actor, destroyed function gonna be called in all clients!
	virtual void Destroyed() override;
	void SpawnTrailSystem();
	void StartDestroyTimer();
	void ExplodeDamage();
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);
	
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* BoxComponent;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticle;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSoundCue;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailNiagaraSystem;
	
	UPROPERTY()
	class UNiagaraComponent* SmokeTrailComponent;
	
	UPROPERTY()
	UParticleSystemComponent* ParticleSystemComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditAnywhere,Category=Damage)
	float DamageValue = 20.f;

	/*
	 * explode damage
	 */
	UPROPERTY(EditAnywhere,Category=Damage)
	float MinDamage = 10.f;
	UPROPERTY(EditAnywhere,Category=Damage)
	float InnerRadius = 200.f;
	UPROPERTY(EditAnywhere,Category=Damage)
	float OuterRadius = 500.f;
	UPROPERTY(EditAnywhere,Category=Damage)
	float Falloff = 1.f;
	
private:
	FTimerHandle DestroyTimer;
	void OnDestroyTimeFinished();
	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.0f;
	
};
