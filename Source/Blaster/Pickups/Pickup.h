// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

class USoundCue;
class USphereComponent;

UCLASS()
class BLASTER_API APickup : public AActor
{
	GENERATED_BODY()

public:
	APickup();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UFUNCTION()
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverLappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere)
	USphereComponent* SphereComponent;
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;
	UPROPERTY(EditAnywhere)
	USoundCue* PickupSound;
	UPROPERTY(EditAnywhere)
	class UNiagaraComponent* PickupEffectComp;
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffect;

	UFUNCTION(NetMulticast, Reliable) // why unreliable is not work?
	void MulticastOnHit(AActor* OtherActor);

	FTimerHandle BindOverlapTimer;
	float BindOverlapTime = 0.25f;
	void BindOverlapTimerFinished();

};
