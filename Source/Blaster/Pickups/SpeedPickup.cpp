// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedPickup.h"

#include "Blaster/BlasterComponents/BuffComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

void ASpeedPickup::OnSphereBeginOverlap(UPrimitiveComponent* OverLappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereBeginOverlap(OverLappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if(OtherActor)
	{
		ABlasterCharacter* OverlapCharacter = Cast<ABlasterCharacter>(OtherActor);
		if(OverlapCharacter && OverlapCharacter->GetBuff())
		{
			OverlapCharacter->GetBuff()->SpeedBuff(WalkBuffSpeed, CrouchBuffSpeed, BuffTime);
		}	
	}
	Destroy();
}

