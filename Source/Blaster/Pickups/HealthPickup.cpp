// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/BlasterComponents/BuffComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

AHealthPickup::AHealthPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;
	PickupEffectComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	PickupEffectComp->SetupAttachment(RootComponent);
	
}

void AHealthPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

void AHealthPickup::Destroyed()
{
	if(PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			PickupEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}
	Super::Destroyed();
}

void AHealthPickup::OnSphereBeginOverlap(UPrimitiveComponent* OverLappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereBeginOverlap(OverLappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	
	if(OtherActor)
	{
		ABlasterCharacter* OverlapCharacter = Cast<ABlasterCharacter>(OtherActor);
		if(OverlapCharacter && OverlapCharacter->GetBuff())
		{
			OverlapCharacter->GetBuff()->Heal(HealAmount, HealTime);
		}	
	}
	Destroy();
}

