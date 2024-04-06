// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/Weapon/RocketProjectileMovement.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

#if WITH_EDITOR
void AProjectileRocket::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName ChangedPropertyName = PropertyChangedEvent.Property->GetFName();
	if(GET_MEMBER_NAME_CHECKED(AProjectileRocket, InitialSpeed) == ChangedPropertyName)
	{
		if(RocketProjectileMovementComponent)
		{
			RocketProjectileMovementComponent->InitialSpeed = InitialSpeed;
			RocketProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif

AProjectileRocket::AProjectileRocket()
{
	PrimaryActorTick.bCanEverTick = false;
	
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh");
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketProjectileMovementComponent = CreateDefaultSubobject<URocketProjectileMovement>("RocketProjectTileMovementComp");
	RocketProjectileMovementComponent->bRotationFollowsVelocity = true;
	RocketProjectileMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if(!HasAuthority())
	{
		BoxComponent->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	}

	SpawnTrailSystem();
	
	if(ProjectileLoopSound && LoopingSoundAttenuation)
	{
		ProjectileLSoundCurLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoopSound,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1,
			1,
			0,
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
		);
		
	}
}

void AProjectileRocket::Destroyed()
{
	AActor::Destroyed();
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	// cause damage
	if(HasAuthority())
	{
		ExplodeDamage();
	}
	
	StartDestroyTimer();
	
	if(ImpactParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, GetActorTransform());
	}
	if(ImpactSoundCue)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSoundCue, GetActorLocation());
	}
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
	if (ProjectileLSoundCurLoopComponent && ProjectileLSoundCurLoopComponent->IsPlaying())
	{
		ProjectileLSoundCurLoopComponent->Stop();
	}
}

