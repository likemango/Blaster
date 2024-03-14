﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"

#include "Blaster/BlasterComponents/BuffComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

AHealthPickup::AHealthPickup()
{
	bReplicates = true;
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
			OverlapCharacter->GetBuff()->HealBuff(HealAmount, HealTime);
		}	
	}
	Destroy();
}

