// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"

#include "NiagaraFunctionLibrary.h"
#include "Blaster/Blaster.h"
#include "Components/BoxComponent.h"
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
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if(Tracer)
	{
		ParticleSystemComponent = UGameplayStatics::SpawnEmitterAttached(
				Tracer,
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

	if(ImpactParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, GetActorTransform());
	}
	if(ImpactSoundCue)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSoundCue, GetActorLocation());
	}
}

void AProjectile::SpawnTrailSystem()
{
	if(TrailNiagaraSystem)
	{
		SmokeTrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		TrailNiagaraSystem,
		GetRootComponent(),
		FName(),
		GetActorLocation(),
		GetActorRotation(),
		EAttachLocation::KeepWorldPosition,
		false);
	}
}

void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
	DestroyTimer,
	this,
	&AProjectile::OnDestroyTimeFinished, DestroyTime);
}

void AProjectile::ExplodeDamage()
{
	APlayerController* InstigatorController = nullptr;
	if(GetInstigator())
	{
		InstigatorController = Cast<APlayerController>(GetInstigator()->GetController());
	}
	// 使用虚幻自带的damage逻辑
	UGameplayStatics::ApplyRadialDamageWithFalloff(
		this,
		DamageValue,
		MinDamage,
		GetActorLocation(),
		InnerRadius,
		OuterRadius,
		Falloff,
		UDamageType::StaticClass(),
		TArray<AActor*>(),
		this,
		InstigatorController
	);
}

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Destroy();
}

void AProjectile::OnDestroyTimeFinished()
{
	Destroy();
}

