// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"


AAmmoPickup::AAmmoPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SphereComponent->AddLocalOffset(FVector(0,0,85));

	MeshComponent->SetRelativeScale3D(FVector(3,3,3));
	MeshComponent->SetRenderCustomDepth(true);
	MeshComponent->CustomDepthStencilValue = CUSTOM_DEPTH_PURPLE;
}

void AAmmoPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	MeshComponent->AddWorldRotation(FRotator(0, RotateRate * DeltaSeconds, 0));
}

void AAmmoPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAmmoPickup::OnSphereBeginOverlap(UPrimitiveComponent* OverLappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereBeginOverlap(OverLappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if(OtherActor)
	{
		ABlasterCharacter* OverlapCharacter = Cast<ABlasterCharacter>(OtherActor);
		if(OverlapCharacter && OverlapCharacter->GetCombat())
		{
			OverlapCharacter->GetCombat()->PickupAmmo(AmmoAmount, AmmoWeaponType);
		}	
	}
	Destroy();
}
