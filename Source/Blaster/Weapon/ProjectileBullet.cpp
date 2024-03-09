﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"

#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"


AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectTileMovementComp");
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
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
