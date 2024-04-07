// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"

#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
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

	/*FPredictProjectilePathParams ProjectilePathParams;
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
	*/
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	// projectileBullet/weapon都是同一个owner即player
	ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(GetOwner());
	if(OwnerCharacter)
	{
		ABlasterPlayerController* Controller = Cast<ABlasterPlayerController>(OwnerCharacter->Controller);
		if(Controller)
		{
			if(OwnerCharacter->HasAuthority() && !bUseServerSideRewind)
			{
				UGameplayStatics::ApplyDamage(OtherActor, DamageValue, Controller, this, UDamageType::StaticClass());
				Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}
			ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor);
			if(bUseServerSideRewind && OwnerCharacter->GetLagCompensation() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
			{
				OwnerCharacter->GetLagCompensation()->ProjectileServerScoreRequest(
					HitCharacter, TraceStart, InitialVelocity, Controller->GetServerTime() - Controller->SingleTripTime);
			}
		}
	}
	
	Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}
