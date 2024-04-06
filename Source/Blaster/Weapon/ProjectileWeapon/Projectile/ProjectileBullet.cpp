// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"

#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"


AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectTileMovementComp");
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName ChangedPropertyName = PropertyChangedEvent.Property->GetFName();
	if(GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed) == ChangedPropertyName)
	{
		if(ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	FPredictProjectilePathParams ProjectilePathParams;
	ProjectilePathParams.bTraceComplex = false;
	ProjectilePathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	ProjectilePathParams.ProjectileRadius = 5.0f;
	ProjectilePathParams.SimFrequency = 30.f;
	ProjectilePathParams.StartLocation = GetActorLocation();
	ProjectilePathParams.ActorsToIgnore.Add(this);
	ProjectilePathParams.TraceChannel = ECollisionChannel::ECC_Visibility;
	ProjectilePathParams.MaxSimTime = 4.f;
	ProjectilePathParams.DrawDebugTime = 5.f;
	ProjectilePathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	ProjectilePathParams.bTraceWithChannel = true;
	ProjectilePathParams.bTraceWithCollision = true;
	
	FPredictProjectilePathResult PredictProjectilePathResult;
	UGameplayStatics::PredictProjectilePath(this, ProjectilePathParams, PredictProjectilePathResult);
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	// prrojectileBullet/weapon都是同一个owner即player
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if(OwnerCharacter)
	{
		AController* Controller = OwnerCharacter->Controller;
		if(Controller)
		{
			// 该方法内部调用TakeDamage
			UGameplayStatics::ApplyDamage(OtherActor, DamageValue, Controller, this, UDamageType::StaticClass());
		}
	}
	
	Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}
