// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"

#include "Blaster/Blaster.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>("BoxCollision");
	SetRootComponent(BoxComponent);

	BoxComponent->SetCollisionObjectType(ECC_WorldDynamic);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	BoxComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	BoxComponent->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Block);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectTileMovementComp");
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if(ParticleSystem)
	{
		ParticleSystemComponent = UGameplayStatics::SpawnEmitterAttached(
				ParticleSystem,
				BoxComponent,
				FName(),
				GetActorLocation(),
				GetActorRotation(),
				EAttachLocation::KeepWorldPosition
			);
	}
	if(HasAuthority())
	{
		BoxComponent->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	}
}

void AProjectile::Destroyed()
{
	Super::Destroyed();

	if(HitParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, GetActorTransform());
	}
	if(HitSoundCue)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), HitSoundCue, GetActorLocation());
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Destroyed();
}

