// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"

#include "NiagaraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


AProjectileGrenade::AProjectileGrenade()
{
	PrimaryActorTick.bCanEverTick = false;
	
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>("GrenadeMesh");
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectTileMovementComp");
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;
}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();

	SpawnTrailSystem();
	StartDestroyTimer();

	// 忽略我本身与发起者的碰撞
	BoxComponent->IgnoreActorWhenMoving(GetInstigator(), true);
	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &ThisClass::OnBounce);
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	UE_LOG(LogTemp, Warning, TEXT("BONCE!!!"));
	if(BoundSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BoundSound,
			GetActorLocation());
	}
}

void AProjectileGrenade::Destroyed()
{
	ExplodeDamage();
	
	if(ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}
	if(BoxComponent)
	{
		BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if(SmokeTrailComponent && SmokeTrailComponent->GetSystemInstanceController())
	{
		SmokeTrailComponent->Deactivate();
	}
	Super::Destroyed();
}
