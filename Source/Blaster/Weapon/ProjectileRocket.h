// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
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
	UStaticMeshComponent* RocketMesh;
	UPROPERTY(VisibleAnywhere)
	class URocketProjectileMovement* RocketProjectileMovementComponent;
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* SmokeTrailNiagara;
	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileLoopSound;
	UPROPERTY()
	UAudioComponent* ProjectileLSoundCurLoopComponent;
	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;

	FTimerHandle SmokeTrailDestroyTimer;
	void OnSmokeTrailDestroyTimeFinished();
	UPROPERTY(EditAnywhere)
	float SmokeTrailDestroyTime = 3.0f;
	UPROPERTY()
	class UNiagaraComponent* SmokeTrailComponent;
	
	UPROPERTY(EditAnywhere,Category=Damage)
	float MinDamage = 10.f;
	UPROPERTY(EditAnywhere,Category=Damage)
	float InnerRadius = 200.f;
	UPROPERTY(EditAnywhere,Category=Damage)
	float OuterRadius = 500.f;
	UPROPERTY(EditAnywhere,Category=Damage)
	float Falloff = 1.f;
};
