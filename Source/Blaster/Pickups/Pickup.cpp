// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


APickup::APickup()
{
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
	SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereCollision");
	SphereComponent->SetupAttachment(RootComponent);

	SphereComponent->SetSphereRadius(150.f);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("PickupMesh");
	MeshComponent->SetupAttachment(SphereComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupEffectComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	PickupEffectComp->SetupAttachment(RootComponent);
}

void APickup::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			BindOverlapTimer,
			this,
			&APickup::BindOverlapTimerFinished,
			BindOverlapTime
		);
	}
}

void APickup::Destroyed()
{
	Super::Destroyed();

	UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());

	// if(PickupEffect)
	// {
	// 	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
	// 		this,
	// 		PickupEffect,
	// 		GetActorLocation(),
	// 		GetActorRotation()
	// 	);
	// }
}

void APickup::OnSphereBeginOverlap(UPrimitiveComponent* OverLappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor)
	{
		MulticastOnHit(OtherActor);
	}
}

void APickup::MulticastOnHit_Implementation(AActor* OtherActor)
{
	if(PickupEffect && OtherActor)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			PickupEffect,
			OtherActor->GetRootComponent(),
			FName(),
			OtherActor->GetActorLocation(),
			OtherActor->GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			true
		);
	}
}

void APickup::BindOverlapTimerFinished()
{
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereBeginOverlap);
}