// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


AProjectileRocket::AProjectileRocket()
{
	PrimaryActorTick.bCanEverTick = false;
	
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh");
	RocketMesh->SetupAttachment(RootComponent);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if(!HasAuthority())
	{
		BoxComponent->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	}
	
	SmokeTrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		SmokeTrailNiagara,
		GetRootComponent(),
		FName(),
		GetActorLocation(),
		GetActorRotation(),
		EAttachLocation::KeepWorldPosition,
		false);

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
	// todo: do nothing here!
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	// cause damage
	if(HasAuthority())
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
	GetWorldTimerManager().SetTimer(
		SmokeTrailDestroyTimer,
		this,
		&AProjectileRocket::OnSmokeTrailDestroyTimeFinished, SmokeTrailDestroyTime);
	
	if(HitParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, GetActorTransform());
	}
	if(HitSoundCue)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), HitSoundCue, GetActorLocation());
	}
	if(RocketMesh)
	{
		RocketMesh->SetVisibility(false);
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

void AProjectileRocket::OnSmokeTrailDestroyTimeFinished()
{
	Destroyed();
}

